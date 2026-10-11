#!/usr/bin/env python3
"""Compile matrix of final_platform_layer.h.

Every job compiles two translation units, one with FPL_IMPLEMENTATION and one that only includes the header,
and links them into an executable. Results go to demos/immediates/fpl_test_runs/<label>/matrix/.

  matrix.py list                               all job ids
  matrix.py run <label> [--only REGEX] [--parallel N]
  matrix.py compare <baseline label> <label>   status changes and FPL warnings that are new or gone
"""
import argparse
import concurrent.futures
import json
import os
import re
import shutil
import subprocess
import sys

TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(os.path.dirname(TOOLS_DIR))
RUNS_DIR = os.path.join(REPO_ROOT, "demos", "immediates", "fpl_test_runs")
HEADER_PATH = os.path.join(REPO_ROOT, "final_platform_layer.h")
WARNING_PATTERN = re.compile(r"final_platform_layer\.h:(\d+):\d+: warning: (.*?)(?: \[(-W[^\]]+)\])?$")
ERROR_PATTERN = re.compile(r"(error:|undefined reference|ld returned|collect2)")
DEFAULT_PARALLEL_JOBS = 16
COMMAND_TIMEOUT_SECONDS = 600

# English compiler messages, so logs of different machines and runs compare as text
os.environ["LC_ALL"] = "C"
os.environ["LANG"] = "C"

WARNING_FLAGS = ["-Wall", "-Wextra"]
DEFAULT_OPTIMIZATION = "-O2"
LINUX_LIBS = ["-ldl", "-lpthread", "-lm"]
LINUX_DIRECT_LIBS = ["-lX11", "-lXrandr", "-lXinerama", "-lXi", "-lGL", "-lvulkan", "-lasound", "-lpulse"]
PIPEWIRE_INCLUDES = ["-I/usr/include/pipewire-0.3", "-I/usr/include/spa-0.2"]
PIPEWIRE_LIBS = ["-lpipewire-0.3"]
WINDOWS_DIRECT_LIBS = ["-lopengl32", "-ldsound", "-lxinput", "-lole32", "-luser32", "-lgdi32", "-lshell32", "-lvulkan-1"]

# name -> (C compiler, C++ compiler, platform, extra flags for every step)
TOOLCHAINS = {
    "gcc": ("gcc", "g++", "linux", []),
    "clang": ("clang", "clang++", "linux", []),
    "gcc-m32": ("gcc", "g++", "linux", ["-m32"]),
    "mingw64": ("x86_64-w64-mingw32-gcc", "x86_64-w64-mingw32-g++", "windows", []),
    "mingw32": ("i686-w64-mingw32-gcc", "i686-w64-mingw32-g++", "windows", []),
}

PRELUDE_SOURCE = """// Included first by both translation units: what a configuration needs from the program itself
#if defined(FPL_NO_CRT)
#	include <stdarg.h>
#	include <stddef.h>
static int MatrixVsnprintf(char *buffer, size_t count, const char *format, va_list args) {
	(void)format;
	(void)args;
	if (count > 0) {
		buffer[0] = 0;
	}
	return 0;
}
#	define FPL_USERFUNC_vsnprintf MatrixVsnprintf
#endif
"""

IMPL_SOURCE = """// Implementation translation unit of the matrix, the configuration comes from the command line
#include "prelude.h"
#define FPL_IMPLEMENTATION
#include <final_platform_layer.h>

#if defined(MATRIX_SINGLE_TU)
#	include "other.c"
#else
extern int MatrixOther(void);
#endif

int main(int argc, char **argv) {
	(void)argc;
	(void)argv;
	bool isInitialized = fplPlatformInit(fplInitFlags_None, fpl_null);
	if (isInitialized) {
		fplPlatformRelease();
	}
	int otherResult = MatrixOther();
	return otherResult;
}
"""

OTHER_SOURCE = """// Header-only translation unit of the matrix: a few API calls and one force-inlined helper
#include "prelude.h"
#include <final_platform_layer.h>

fpl_force_inline int MatrixInlineHelper(const int value) {
	return value + 1;
}

int MatrixOther(void) {
	const char *version = fplGetVersion();
	size_t length = fplGetStringLength(version);
	int helped = MatrixInlineHelper(0);
	bool isValid = (length > 0) && (helped == 1);
	return isValid ? 0 : 1;
}
"""


def make_job(toolchain, language, standard, defines=(), extra=(), libs=(), mode="two_tu", optimization=DEFAULT_OPTIMIZATION, tag=""):
    define_part = "+".join(define.replace("FPL_", "") for define in defines) or "default"
    job_id = f"{toolchain}-{standard}-{define_part}"
    if tag:
        job_id += f"-{tag}"
    if optimization != DEFAULT_OPTIMIZATION:
        job_id += f"{optimization}"
    return {
        "id": job_id,
        "toolchain": toolchain,
        "language": language,
        "standard": standard,
        "defines": list(defines),
        "extra": list(extra),
        "libs": list(libs),
        "mode": mode,
        "optimization": optimization,
    }


def all_jobs():
    jobs = []
    # Default configuration with every toolchain and standard
    for standard in ["c99", "c11", "c17", "gnu99"]:
        jobs.append(make_job("gcc", "c", standard))
    for standard in ["c99", "c11", "c17"]:
        jobs.append(make_job("clang", "c", standard))
    for standard in ["c++11", "c++17", "c++20"]:
        jobs.append(make_job("gcc", "c++", standard))
    for standard in ["c++11", "c++20"]:
        jobs.append(make_job("clang", "c++", standard))
    jobs.append(make_job("gcc-m32", "c", "c99"))
    jobs.append(make_job("gcc-m32", "c++", "c++11"))
    jobs.append(make_job("gcc-m32", "c", "c99", defines=["FPL_NO_VIDEO_VULKAN"]))
    for toolchain in ["mingw64", "mingw32"]:
        jobs.append(make_job(toolchain, "c", "c99"))
        jobs.append(make_job(toolchain, "c", "c11"))
        jobs.append(make_job(toolchain, "c++", "c++11"))
        jobs.append(make_job(toolchain, "c++", "c++17"))
    # Unoptimized builds, a force-inlined function without an external definition only fails here
    jobs.append(make_job("gcc", "c", "c99", optimization="-O0"))
    jobs.append(make_job("mingw64", "c", "c99", optimization="-O0"))
    jobs.append(make_job("mingw32", "c", "c99", optimization="-O0"))
    # Pedantic, for information
    jobs.append(make_job("gcc", "c", "c99", extra=["-Wpedantic"], tag="pedantic"))
    jobs.append(make_job("clang", "c", "c99", extra=["-Wpedantic"], tag="pedantic"))
    jobs.append(make_job("gcc", "c++", "c++11", extra=["-Wpedantic"], tag="pedantic"))
    # Every switch on its own
    common_switches = [
        ["FPL_NO_WINDOW"], ["FPL_NO_VIDEO"], ["FPL_NO_VIDEO_OPENGL"], ["FPL_NO_VIDEO_VULKAN"], ["FPL_NO_VIDEO_SOFTWARE"],
        ["FPL_NO_AUDIO"], ["FPL_NO_INPUT"], ["FPL_LOGGING"], ["FPL_LOGGING", "FPL_LOG_MULTIPLE_WRITERS"],
        ["FPL_NO_ASSERTIONS"], ["FPL_FORCE_ASSERTIONS"], ["FPL_NO_C_ASSERT"], ["FPL_NO_UNDEF"],
        ["FPL_NO_MEMORY_MACROS"], ["FPL_FORCE_MEMORY_MACROS"], ["FPL_DEBUG"], ["FPL_RELEASE"],
        ["FPL_CRASH_ON_ERROR"], ["FPL_CRASH_ON_WARNING"], ["FPL_MULTI_TRANSLATION_UNIT"], ["FPL_NO_PLATFORM_INCLUDES"],
    ]
    linux_switches = [
        ["FPL_NO_AUDIO_ALSA"], ["FPL_NO_AUDIO_PULSEAUDIO"], ["FPL_NO_AUDIO_PIPEWIRE"], ["FPL_NO_AUDIO_OSS"],
        ["FPL_NO_INPUT_X11"], ["FPL_NO_INPUT_LINUX_JOYSTICK"], ["FPL_NO_INPUT_UNIX_GAMEPAD"], ["FPL_NO_X11_XINPUT2"],
    ]
    windows_switches = [
        ["FPL_NO_AUDIO_WASAPI"], ["FPL_NO_AUDIO_DIRECTSOUND"], ["FPL_NO_INPUT_XINPUT"], ["FPL_NO_INPUT_DINPUT"],
        ["FPL_NO_INPUT_RAWINPUT"], ["FPL_NO_INPUT_WIN32"],
    ]
    for switches in common_switches + linux_switches:
        jobs.append(make_job("gcc", "c", "c99", defines=switches))
        jobs.append(make_job("gcc", "c++", "c++11", defines=switches))
    for switches in common_switches + windows_switches:
        jobs.append(make_job("mingw64", "c", "c99", defines=switches))
    # FPL_API_AS_PRIVATE only works in a single translation unit
    for toolchain, language, standard in [("gcc", "c", "c99"), ("gcc", "c++", "c++11"), ("mingw64", "c", "c99")]:
        jobs.append(make_job(toolchain, language, standard, defines=["FPL_API_AS_PRIVATE"], mode="single_tu"))
    # Direct linking of the system libraries
    for language, standard in [("c", "c99"), ("c++", "c++11")]:
        jobs.append(make_job("gcc", language, standard, defines=["FPL_NO_RUNTIME_LINKING"], libs=LINUX_DIRECT_LIBS))
        jobs.append(make_job("gcc", language, standard, defines=["FPL_NO_RUNTIME_LINKING"], extra=PIPEWIRE_INCLUDES, libs=LINUX_DIRECT_LIBS + PIPEWIRE_LIBS, tag="pipewire-headers"))
        jobs.append(make_job("gcc", language, standard, defines=["FPL_NO_RUNTIME_LINKING", "FPL_NO_PLATFORM_INCLUDES"], libs=LINUX_DIRECT_LIBS))
    jobs.append(make_job("mingw64", "c", "c99", defines=["FPL_NO_RUNTIME_LINKING"], libs=WINDOWS_DIRECT_LIBS))
    jobs.append(make_job("mingw64", "c", "c99", defines=["FPL_NO_RUNTIME_LINKING", "FPL_NO_VIDEO_VULKAN"], libs=WINDOWS_DIRECT_LIBS[:-1]))
    # What real programs use: console only, window only
    console_only = ["FPL_NO_WINDOW", "FPL_NO_VIDEO", "FPL_NO_AUDIO"]
    for toolchain, language, standard in [("gcc", "c", "c99"), ("clang", "c", "c99"), ("gcc", "c++", "c++11"), ("mingw64", "c", "c99"), ("gcc-m32", "c", "c99")]:
        jobs.append(make_job(toolchain, language, standard, defines=console_only))
    window_only = ["FPL_NO_VIDEO", "FPL_NO_AUDIO"]
    for toolchain in ["gcc", "mingw64"]:
        jobs.append(make_job(toolchain, "c", "c99", defines=window_only))
    # No C runtime on Windows, compiled only because the entry and the memory functions have to come from the program
    jobs.append(make_job("mingw64", "c", "c99", defines=["FPL_NO_CRT", "FPL_APPTYPE_CONSOLE"], mode="compile_only"))
    jobs.append(make_job("mingw64", "c++", "c++11", defines=["FPL_NO_CRT", "FPL_APPTYPE_CONSOLE"], mode="compile_only"))
    # C++98 is below the minimum, it has to fail with a clear message
    jobs.append(make_job("gcc", "c++", "c++98"))
    return jobs


def run_command(command, log_file, cwd):
    log_file.write("$ " + " ".join(command) + "\n")
    log_file.flush()
    completed = subprocess.run(command, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, errors="replace", timeout=COMMAND_TIMEOUT_SECONDS)
    log_file.write(completed.stdout)
    log_file.write(f"### EXIT {completed.returncode}\n")
    return completed.returncode


def run_job(job, job_dir, source_dir):
    os.makedirs(job_dir, exist_ok=True)
    c_compiler, cxx_compiler, platform, toolchain_flags = TOOLCHAINS[job["toolchain"]]
    is_cxx = job["language"] == "c++"
    compiler = cxx_compiler if is_cxx else c_compiler
    language_flags = ["-x", "c++"] if is_cxx else []
    define_flags = [f"-D{define}" for define in job["defines"]]
    if job["mode"] == "single_tu":
        define_flags.append("-DMATRIX_SINGLE_TU")
    compile_flags = toolchain_flags + [f"-std={job['standard']}", job["optimization"]] + WARNING_FLAGS + job["extra"] + define_flags + [f"-I{REPO_ROOT}", f"-I{source_dir}"]
    platform_libs = LINUX_LIBS if platform == "linux" else []
    units = ["impl.c"] if job["mode"] == "single_tu" else ["impl.c", "other.c"]
    status = "OK"
    log_path = os.path.join(job_dir, "build.log")
    with open(log_path, "w", encoding="utf-8") as log_file:
        objects = []
        for unit in units:
            object_path = os.path.join(job_dir, unit.replace(".c", ".o"))
            command = [compiler] + language_flags + compile_flags + ["-c", os.path.join(source_dir, unit), "-o", object_path]
            if run_command(command, log_file, job_dir) != 0:
                status = "CC-ERR"
            objects.append(object_path)
        if status == "OK" and job["mode"] != "compile_only":
            executable = os.path.join(job_dir, "matrix.exe" if platform == "windows" else "matrix")
            command = [compiler] + toolchain_flags + objects + ["-o", executable] + job["libs"] + platform_libs
            if run_command(command, log_file, job_dir) != 0:
                status = "LD-ERR"
    return status, log_path


def parse_log(log_path, header_lines):
    fpl_warnings = set()
    warning_count = 0
    first_error = ""
    with open(log_path, encoding="utf-8", errors="replace") as log_file:
        for raw_line in log_file:
            line = raw_line.rstrip()
            if "warning:" in line:
                warning_count += 1
                match = WARNING_PATTERN.search(line)
                if match:
                    line_number = int(match.group(1))
                    source = header_lines[line_number - 1].strip() if line_number <= len(header_lines) else "?"
                    fpl_warnings.add((match.group(3) or "-", source, match.group(2), line_number))
            elif not first_error and ERROR_PATTERN.search(line) and not line.startswith("$ "):
                first_error = line[:300]
    return warning_count, fpl_warnings, first_error


def command_run(label, only, parallel):
    run_dir = os.path.join(RUNS_DIR, label)
    matrix_dir = os.path.join(run_dir, "matrix")
    source_dir = os.path.join(matrix_dir, "src")
    # A partial run keeps the results of every other job
    results_path = os.path.join(matrix_dir, "results.json")
    results = {}
    if only and os.path.exists(results_path):
        with open(results_path, encoding="utf-8") as results_file:
            results = json.load(results_file)
    else:
        shutil.rmtree(matrix_dir, ignore_errors=True)
    os.makedirs(source_dir, exist_ok=True)
    with open(os.path.join(source_dir, "prelude.h"), "w", encoding="utf-8") as source_file:
        source_file.write(PRELUDE_SOURCE)
    with open(os.path.join(source_dir, "impl.c"), "w", encoding="utf-8") as source_file:
        source_file.write(IMPL_SOURCE)
    with open(os.path.join(source_dir, "other.c"), "w", encoding="utf-8") as source_file:
        source_file.write(OTHER_SOURCE)
    # Always the header this run compiles, every result keeps the source text of its warnings, so older results stay valid
    header_copy = os.path.join(run_dir, "final_platform_layer.h")
    shutil.copy(HEADER_PATH, header_copy)
    with open(header_copy, encoding="utf-8", errors="replace") as header_file:
        header_lines = header_file.read().split("\n")
    known_ids = {job["id"] for job in all_jobs()}
    results = {job_id: result for job_id, result in results.items() if job_id in known_ids}
    jobs = [job for job in all_jobs() if not only or re.search(only, job["id"])]
    with concurrent.futures.ThreadPoolExecutor(max_workers=parallel) as executor:
        futures = {executor.submit(run_job, job, os.path.join(matrix_dir, "jobs", job["id"]), source_dir): job for job in jobs}
        for future in concurrent.futures.as_completed(futures):
            job = futures[future]
            status, log_path = future.result()
            warning_count, fpl_warnings, first_error = parse_log(log_path, header_lines)
            results[job["id"]] = {
                "job": job,
                "status": status,
                "warnings": warning_count,
                "fpl_warnings": sorted(fpl_warnings),
                "first_error": first_error,
            }
    with open(results_path, "w", encoding="utf-8") as results_file:
        json.dump(results, results_file, indent=1)
    failed = sorted(job_id for job_id, result in results.items() if result["status"] != "OK")
    print(f"{label}: {len(results)} jobs, {len(failed)} not OK")
    for job_id in failed:
        print(f"  {results[job_id]['status']:7} {job_id}: {results[job_id]['first_error']}")
    distinct_sites = {(flag, source) for result in results.values() for (flag, source, message, line_number) in result["fpl_warnings"]}
    print(f"  distinct FPL warning sites over all jobs: {len(distinct_sites)}")


def load_results(label):
    with open(os.path.join(RUNS_DIR, label, "matrix", "results.json"), encoding="utf-8") as results_file:
        return json.load(results_file)


def command_compare(baseline_label, label):
    baseline = load_results(baseline_label)
    current = load_results(label)
    print(f"=== job status ({baseline_label} -> {label})")
    for job_id in sorted(set(baseline) | set(current)):
        before = baseline.get(job_id, {}).get("status", "-")
        after = current.get(job_id, {}).get("status", "-")
        before_sites = len(baseline.get(job_id, {}).get("fpl_warnings", []))
        after_sites = len(current.get(job_id, {}).get("fpl_warnings", []))
        if before != after or before_sites != after_sites:
            print(f"  {job_id}: {before} -> {after}, FPL warnings {before_sites} -> {after_sites}")
    baseline_sites = {(flag, source) for result in baseline.values() for (flag, source, message, line_number) in result["fpl_warnings"]}
    current_sites = {}
    for job_id, result in current.items():
        for (flag, source, message, line_number) in result["fpl_warnings"]:
            current_sites.setdefault((flag, source), []).append((job_id, message, line_number))
    new_keys = sorted(set(current_sites) - baseline_sites)
    gone_count = len(baseline_sites - set(current_sites))
    print(f"=== FPL warning sites: {len(baseline_sites)} -> {len(current_sites)}, new {len(new_keys)}, gone {gone_count}")
    for key in new_keys:
        hits = current_sites[key]
        print(f"  NEW [{key[0]}] line {sorted({hit[2] for hit in hits})} in {len(hits)} jobs, e.g. {hits[0][0]}: {hits[0][1]}\n      {key[1][:160]}")


def main():
    parser = argparse.ArgumentParser(description="Compile matrix of final_platform_layer.h")
    subparsers = parser.add_subparsers(dest="command", required=True)
    subparsers.add_parser("list")
    run_parser = subparsers.add_parser("run")
    run_parser.add_argument("label")
    run_parser.add_argument("--only", default="")
    run_parser.add_argument("--parallel", type=int, default=DEFAULT_PARALLEL_JOBS)
    compare_parser = subparsers.add_parser("compare")
    compare_parser.add_argument("baseline")
    compare_parser.add_argument("label")
    arguments = parser.parse_args()
    if arguments.command == "list":
        for job in all_jobs():
            print(job["id"])
    elif arguments.command == "run":
        command_run(arguments.label, arguments.only, arguments.parallel)
    else:
        command_compare(arguments.baseline, arguments.label)


if __name__ == "__main__":
    main()
