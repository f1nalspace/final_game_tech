#!/usr/bin/env bash
# Builds FPL_ImageViewer as the standalone app finalimageviewer (CMake option FINALIMAGEVIEWER_APP) into demos/build/finalimageviewer/finalimageviewer-<version>-x86_64.AppImage.
# The version is taken from version.h. The binary links libc, libm and libdl only (libstdc++ and libgcc are linked statically), X11 and OpenGL are loaded at runtime,
# the fonts are compiled in, so the AppImage carries nothing but the binary, its desktop file and its icon.
# libwebp is not bundled either, the one of the system is loaded at runtime when present; without it WebP files cannot be read.
set -euo pipefail

scriptDirectory="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
projectDirectory="$(cd "$scriptDirectory/../.." && pwd)"
demosDirectory="$(cd "$projectDirectory/.." && pwd)"
outputDirectory="$demosDirectory/build/finalimageviewer"
cmakeDirectory="$outputDirectory/cmake"
appDirectory="$outputDirectory/AppDir"
toolsDirectory="$outputDirectory/tools"
appName="finalimageviewer"

# Optimized, but with debug info, so a crash in daily use can still be read with gdb
buildType="RelWithDebInfo"
iconPixelSize=256

# Pinned versions with their checksums, so a build never picks up a tool that changed under the same name
appimagetoolVersion="1.9.1"
appimagetoolUrl="https://github.com/AppImage/appimagetool/releases/download/$appimagetoolVersion/appimagetool-x86_64.AppImage"
appimagetoolSha256="ed4ce84f0d9caff66f50bcca6ff6f35aae54ce8135408b3fa33abfc3cb384eb0"
appimagetoolPath="$toolsDirectory/appimagetool-$appimagetoolVersion-x86_64.AppImage"
runtimeVersion="20251108"
runtimeUrl="https://github.com/AppImage/type2-runtime/releases/download/$runtimeVersion/runtime-x86_64"
runtimeSha256="2fca8b443c92510f1483a883f60061ad09b46b978b2631c807cd873a47ec260d"
runtimePath="$toolsDirectory/runtime-$runtimeVersion-x86_64"

# Downloads a tool once and refuses it when its checksum is not the pinned one
fetchTool() {
	local url="$1"
	local expectedSha256="$2"
	local targetPath="$3"
	if [ ! -f "$targetPath" ]; then
		echo "Downloading $url"
		curl --fail --location --silent --show-error --output "$targetPath.part" "$url"
		mv "$targetPath.part" "$targetPath"
	fi
	local actualSha256
	actualSha256="$(sha256sum "$targetPath" | cut -d ' ' -f 1)"
	if [ "$actualSha256" != "$expectedSha256" ]; then
		echo "Checksum of $targetPath does not match, delete it and try again" >&2
		exit 1
	fi
	chmod +x "$targetPath"
}

mkdir -p "$toolsDirectory"
fetchTool "$appimagetoolUrl" "$appimagetoolSha256" "$appimagetoolPath"
fetchTool "$runtimeUrl" "$runtimeSha256" "$runtimePath"

cmake -S "$projectDirectory" -B "$cmakeDirectory" -DCMAKE_BUILD_TYPE="$buildType" -DFINALIMAGEVIEWER_APP=ON
cmake --build "$cmakeDirectory" --parallel

# The CMake project writes its executable next to the other demos, see THIS_EXECUTABLE_PATH in CMakeLists.txt
executablePath="$demosDirectory/build/FPL_ImageViewer/Linux-x64-$buildType/$appName"
if [ ! -x "$executablePath" ]; then
	echo "No executable at $executablePath" >&2
	exit 1
fi

version="$(sed -n 's/^#define VER_PRODUCTVERSION_STR[[:space:]]*"\([0-9.]*\).*/\1/p' "$projectDirectory/version.h")"
if [ -z "$version" ]; then
	echo "No VER_PRODUCTVERSION_STR in $projectDirectory/version.h" >&2
	exit 1
fi
appImagePath="$outputDirectory/$appName-$version-x86_64.AppImage"

rm -rf "$appDirectory"
mkdir -p "$appDirectory/usr/bin"
mkdir -p "$appDirectory/usr/share/applications"
mkdir -p "$appDirectory/usr/share/icons/hicolor/scalable/apps"
install -m 755 "$executablePath" "$appDirectory/usr/bin/$appName"
install -m 644 "$scriptDirectory/$appName.svg" "$appDirectory/usr/share/icons/hicolor/scalable/apps/$appName.svg"

# Gear Lever shows and compares the version it finds under X-AppImage-Version in the desktop file
desktopFilePath="$appDirectory/usr/share/applications/$appName.desktop"
install -m 644 "$scriptDirectory/$appName.desktop" "$desktopFilePath"
echo "X-AppImage-Version=$version" >> "$desktopFilePath"

# appimagetool wants the desktop file and the icon in the root as well, and .DirIcon is what file managers show for the AppImage
cp "$desktopFilePath" "$appDirectory/$appName.desktop"
cp "$scriptDirectory/$appName.svg" "$appDirectory/$appName.svg"
rsvg-convert --width "$iconPixelSize" --height "$iconPixelSize" "$scriptDirectory/$appName.svg" --output "$appDirectory/.DirIcon"
ln -s "usr/bin/$appName" "$appDirectory/AppRun"

ARCH=x86_64 "$appimagetoolPath" --no-appstream --runtime-file "$runtimePath" "$appDirectory" "$appImagePath"
echo "Built $appImagePath"
