#!/usr/bin/env python3
# Evaluates the logs of build_all.sh.
#   compare_builds.py --summary <label>          table of one run, also written to results.tsv
#   compare_builds.py <baseline label> <label>   per project changes and the FPL warnings that are new or gone
# FPL warnings are compared by flag and source text of the warned line, because line numbers move with every change.
import collections
import glob
import os
import re
import sys

TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))
RUNS_DIR = os.path.join(os.path.dirname(os.path.dirname(TOOLS_DIR)), "demos", "immediates", "fpl_test_runs")
WARNING_PATTERN = re.compile(r"final_platform_layer\.h:(\d+):\d+: warning: (.*?)(?: \[(-W[^\]]+)\])?$")
EXIT_PATTERN = re.compile(r"### (CONFIGURE|BUILD) EXIT (\d+)")
NOT_RUN = "-"


def load_run(label):
    run_dir = os.path.join(RUNS_DIR, label)
    header_path = os.path.join(run_dir, "final_platform_layer.h")
    with open(header_path, encoding="utf-8", errors="replace") as header_file:
        header_lines = header_file.read().split("\n")
    projects = {}
    for log_path in sorted(glob.glob(os.path.join(run_dir, "build_logs", "*.log"))):
        name = os.path.basename(log_path)[:-len(".log")]
        exits = {"CONFIGURE": NOT_RUN, "BUILD": NOT_RUN}
        warning_count = 0
        fpl_sites = set()
        with open(log_path, encoding="utf-8", errors="replace") as log_file:
            for raw_line in log_file:
                line = raw_line.rstrip()
                exit_match = EXIT_PATTERN.search(line)
                if exit_match:
                    exits[exit_match.group(1)] = exit_match.group(2)
                    continue
                if "warning:" not in line:
                    continue
                warning_count += 1
                warning_match = WARNING_PATTERN.search(line)
                if warning_match:
                    line_number = int(warning_match.group(1))
                    message = warning_match.group(2)
                    flag = warning_match.group(3) or "-"
                    source = header_lines[line_number - 1].strip() if line_number <= len(header_lines) else "?"
                    fpl_sites.add((flag, source, message, line_number))
        projects[name] = {
            "configure": exits["CONFIGURE"],
            "build": exits["BUILD"],
            "warnings": warning_count,
            "fpl_sites": fpl_sites,
        }
    return projects


def fpl_site_keys(project):
    return {(flag, source) for (flag, source, message, line_number) in project["fpl_sites"]}


def print_summary(label):
    projects = load_run(label)
    rows = []
    for name, project in sorted(projects.items(), key=lambda item: item[0].lower()):
        rows.append(f"{name}\t{project['configure']}\t{project['build']}\t{project['warnings']}\t{len(project['fpl_sites'])}")
    header = "project\tconfigure\tbuild\twarnings\tfpl_warning_sites"
    with open(os.path.join(RUNS_DIR, label, "results.tsv"), "w", encoding="utf-8") as results_file:
        results_file.write(header + "\n" + "\n".join(rows) + "\n")
    failed = [name for name, project in projects.items() if project["build"] != "0" or project["configure"] != "0"]
    all_sites = set()
    for project in projects.values():
        all_sites |= fpl_site_keys(project)
    print(f"{label}: {len(projects)} projects, failed: {', '.join(sorted(failed)) or 'none'}, distinct FPL warning sites: {len(all_sites)}")
    for row in rows:
        print("  " + row.replace("\t", "  "))


def print_comparison(baseline_label, label):
    baseline = load_run(baseline_label)
    current = load_run(label)
    print(f"=== projects ({baseline_label} -> {label})")
    for name in sorted(set(baseline) | set(current), key=str.lower):
        before = baseline.get(name)
        after = current.get(name)
        if before is None or after is None:
            print(f"  {name}: only in {'current' if before is None else 'baseline'}")
            continue
        changed_result = (before["configure"], before["build"]) != (after["configure"], after["build"])
        warning_delta = after["warnings"] - before["warnings"]
        fpl_delta = len(after["fpl_sites"]) - len(before["fpl_sites"])
        if changed_result or warning_delta != 0 or fpl_delta != 0:
            print(f"  {name}: build {before['build']}->{after['build']}, warnings {before['warnings']}->{after['warnings']}, FPL sites {len(before['fpl_sites'])}->{len(after['fpl_sites'])}")
    baseline_sites = collections.defaultdict(set)
    current_sites = collections.defaultdict(set)
    for name, project in baseline.items():
        for (flag, source, message, line_number) in project["fpl_sites"]:
            baseline_sites[(flag, source)].add(name)
    for name, project in current.items():
        for (flag, source, message, line_number) in project["fpl_sites"]:
            current_sites[(flag, source)].add((name, message, line_number))
    new_keys = sorted(set(current_sites) - set(baseline_sites))
    gone_keys = sorted(set(baseline_sites) - set(current_sites))
    print(f"=== FPL warning sites: {len(baseline_sites)} -> {len(current_sites)}, new {len(new_keys)}, gone {len(gone_keys)}")
    for key in new_keys:
        hits = current_sites[key]
        names = sorted({name for (name, message, line_number) in hits})
        lines = sorted({line_number for (name, message, line_number) in hits})
        message = sorted(hits)[0][1]
        print(f"  NEW [{key[0]}] line {lines} in {len(names)} projects: {message}\n      {key[1][:160]}")


def main():
    arguments = sys.argv[1:]
    if len(arguments) == 2 and arguments[0] == "--summary":
        print_summary(arguments[1])
    elif len(arguments) == 2:
        print_comparison(arguments[0], arguments[1])
    else:
        print(__doc__ or "usage: compare_builds.py --summary <label> | <baseline label> <label>")
        sys.exit(1)


if __name__ == "__main__":
    main()
