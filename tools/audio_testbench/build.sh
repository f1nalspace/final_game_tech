#!/bin/bash
# Builds FPL_AudioPlayer in the test bench variants, one directory per variant below bin/.
# Usage: build.sh [variant ...]   (default: all)
# Variants: gcc-x64-o2 gcc-x64-o0 clang-x64-o2 gcc-x86-o2 asan tsan mingw32-xp-o2 mingw32-xp-o0 (tsan is not part of the default list)
# Each variant writes bin/<variant>/build.log; the summary line counts errors and warnings.

testbench_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "$testbench_dir/../.." && pwd)"
player_source="$repo_root/demos/FPL_AudioPlayer/fpl_audioplayer.c"

export LC_ALL=C
export LANG=C

all_variants=(gcc-x64-o2 gcc-x64-o0 clang-x64-o2 gcc-x86-o2 asan mingw32-xp-o2 mingw32-xp-o0)
if [ $# -gt 0 ]; then
	variants=("$@")
else
	variants=("${all_variants[@]}")
fi

# Same defines and include paths as demos/FPL_AudioPlayer/CMakeLists.txt
defines=(-DFPL_IMPLEMENTATION -DFPL_LOGGING -DFINAL_AUDIOSYSTEM_IMPLEMENTATION -DFINAL_GRAPHICS_IMPLEMENTATION -DFINAL_WAVELOADER_IMPLEMENTATION -DMA_USE_RUNTIME_LINKING_FOR_PTHREAD)
includes=(-I "$repo_root" -I "$repo_root/demos/additions" -I "$repo_root/demos/dependencies")
warnings=(-Wall -Wextra)
linux_libs=(-lm -ldl -lpthread)
xp_flags=(-march=prescott -mfpmath=sse -mcrtdll=msvcrt-os -D_WIN32_WINNT=0x0501 -DWINVER=0x0501 -D__USE_MINGW_ANSI_STDIO=1 -DSTB_VORBIS_NO_STDIO -static)

build_variant() {
	local variant=$1
	local output=$2
	case "$variant" in
		gcc-x64-o2) gcc -std=c99 -g -O2 "${warnings[@]}" "${defines[@]}" "${includes[@]}" "$player_source" -o "$output" "${linux_libs[@]}" ;;
		gcc-x64-o0) gcc -std=c99 -g -O0 "${warnings[@]}" "${defines[@]}" "${includes[@]}" "$player_source" -o "$output" "${linux_libs[@]}" ;;
		clang-x64-o2) clang -std=c99 -g -O2 "${warnings[@]}" "${defines[@]}" "${includes[@]}" "$player_source" -o "$output" "${linux_libs[@]}" ;;
		gcc-x86-o2) gcc -m32 -std=c99 -g -O2 -march=prescott -mfpmath=sse "${warnings[@]}" "${defines[@]}" "${includes[@]}" "$player_source" -o "$output" "${linux_libs[@]}" -latomic ;;
		asan) gcc -std=c99 -g -O1 -fno-omit-frame-pointer -fsanitize=address,undefined "${warnings[@]}" "${defines[@]}" "${includes[@]}" "$player_source" -o "$output" "${linux_libs[@]}" ;;
		tsan) gcc -std=c99 -g -O1 -fno-omit-frame-pointer -fsanitize=thread "${warnings[@]}" "${defines[@]}" "${includes[@]}" "$player_source" -o "$output" "${linux_libs[@]}" ;;
		mingw32-xp-o2) i686-w64-mingw32-gcc -std=c99 -g -O2 "${xp_flags[@]}" "${warnings[@]}" "${defines[@]}" "${includes[@]}" "$player_source" -o "$output" ;;
		mingw32-xp-o0) i686-w64-mingw32-gcc -std=c99 -g -O0 "${xp_flags[@]}" "${warnings[@]}" "${defines[@]}" "${includes[@]}" "$player_source" -o "$output" ;;
		*) echo "unknown variant $variant" >&2; return 1 ;;
	esac
}

for variant in "${variants[@]}"; do
	variant_dir="$testbench_dir/bin/$variant"
	mkdir -p "$variant_dir"
	executable="$variant_dir/FPL_AudioPlayer"
	if [[ "$variant" == mingw* ]]; then
		executable="$variant_dir/FPL_AudioPlayer.exe"
	fi
	rm -f "$executable"
	start_seconds=$SECONDS
	build_variant "$variant" "$executable" > "$variant_dir/build.log" 2>&1
	build_exit=$?
	elapsed_seconds=$((SECONDS - start_seconds))
	error_count=$(grep -c "error:" "$variant_dir/build.log")
	warning_count=$(grep -c "warning:" "$variant_dir/build.log")
	echo "$variant: exit $build_exit, errors $error_count, warnings $warning_count, ${elapsed_seconds}s"
done
