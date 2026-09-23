#!/bin/bash
# Renders the synthetic test images with FPL_ImageViewer --render-to at many scales and with every filter,
# compares each result against an ImageMagick reference of the same kernel computed in linear light and prints a table.
# Where the viewer reads a reduced level (scale <= 0.125), the same picture is also rendered from level 0 (--lod-source=0) and both are compared.
# Exit code is 0 when every threshold holds, 1 when at least one is violated, 2 on usage or setup errors.
#
# Usage: run_scaling_tests.sh [--viewer=<path>] [--images=<name,...>] [--filters=<key,...>] [--quick] [--no-photo]
#   --viewer   FPL_ImageViewer executable (default: Release build, then Debug build under demos/build/FPL_ImageViewer)
#   --images   only these test images, names without extension (e.g. zoneplate_2048,checker_1px)
#   --filters  only these viewer filters, keys as in --down-filter= (nearest box bilinear triangular bell bspline mitchell catmullrom lanczos3)
#   --quick    fewer scales per image
#   --no-photo skip the real photo comparison
# Results (renders, references, report.md) go to demos/build/FPL_ImageViewer/tests, which is ignored by git.

set -uo pipefail

scriptDirectory="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
demosDirectory="$(cd "$scriptDirectory/../.." && pwd)"
imagesDirectory="$scriptDirectory/images"
outputDirectory="$demosDirectory/build/FPL_ImageViewer/tests"
rendersDirectory="$outputDirectory/renders"
referencesDirectory="$outputDirectory/references"
reportFile="$outputDirectory/report.md"
photoFile="/home/final/Bilder/202308/IMG_8978.JPG"

# --- Thresholds (calibrated in iteration 0, see plans/fpl_imageviewer_plan.md section 4.2) ---------------------------------

# Minimum PSNR in dB against the ImageMagick reference of the same kernel; rounding alone limits a correct 8 bit result to >= 54.4 dB, the rest is room for GPU arithmetic
psnrThreshold=45
# Allowed ring standard deviation (8 bit steps) above the reference of the same kernel
ringTolerance=1.0
# Allowed deviation (8 bit steps) of a flat downscaled area from its expected value
flatMeanTolerance=1
# Allowed standard deviation (8 bit steps) inside a flat downscaled area
flatStdTolerance=1
# Allowed deviation (8 bit steps) of gamma_rows and color_checker_1px from 188
gammaTolerance=2
# Allowed asymmetry (8 bit steps) of the upscaled impulse and deviation from the sampled kernel
impulseTolerance=1
# Allowed deviation (8 bit steps) at 100 % of a picture with transparency: blended pixels go through the sRGB encoding of the GPU, which rounds a step differently now and then
translucentIdentityTolerance=1
# Minimum PSNR in dB between the render from the reduced level and the render from level 0, the double filtering must stay invisible (plan section 2.3)
levelPsnrThreshold=45
# Allowed difference (8 bit steps) of the zone plate ring between the render from the reduced level and the one from level 0
levelRingTolerance=0.3
# Scales up to this one read a reduced level: the source level is the smallest one that is still at least eight times the displayed size
largestLevelScale=0.125
# Minimum mean red along each side of border_frame_odd, as a fraction of the same side in the reference; a cut off frame side stays far below
frameMinimumFraction=0.5
# Expected sRGB value of a 50 % linear mix of black and white (and of a 50 % red / green mix per channel)
linearHalfValue=188
# The flat, gamma and mix checks demand a result some kernels cannot reach at some scales (Box at 0.7 leaves the checker board standing, in ImageMagick just as well).
# Such a row passes when the viewer is as close as this (8 bit steps) to the same measurement on the reference of the same kernel, and is marked as a kernel property.
kernelPropertyTolerance=1
# Threshold of the autonearest check (--nearest-from=<percent>): from this zoom on the render must be Nearest, below it the chosen filter
autoNearestPercent=400

# --- Ring metric for the zone plate --------------------------------------------------------------------------------------

# The ring starts at this multiple of the output Nyquist radius (a correct filter is flat there) ...
ringStartNyquistFactor=1.5
# ... and ends at this fraction of the radius where the source itself reaches Nyquist
ringEndSourceNyquistFraction=0.95
# Rings thinner than this (output pixels) are not measured
ringMinimumWidth=4

# --- Viewer filters (keys as --down-filter= / --up-filter=) and their ImageMagick equivalents ------------------------------

filterKeys=(nearest box bilinear triangular bell bspline mitchell catmullrom lanczos3)
declare -A filterNames=([nearest]="Nearest" [box]="Box" [bilinear]="Bilinear" [triangular]="Bicubic (Triangular)" [bell]="Bicubic (Bell)" [bspline]="Bicubic (B-Spline)" [mitchell]="Mitchell" [catmullrom]="Catmull-Rom" [lanczos3]="Lanczos3")
# Triangular is a triangle of radius 2 and Bell a quadratic B-spline stretched to radius 2, blur widens the ImageMagick kernel accordingly
declare -A filterReferenceOptions=([nearest]="-filter Point" [box]="-filter Box" [bilinear]="-filter Triangle" [triangular]="-filter Triangle -define filter:blur=2" [bell]="-filter Quadratic -define filter:blur=1.3333333333" [bspline]="-filter Spline" [mitchell]="-filter Mitchell" [catmullrom]="-filter Catrom" [lanczos3]="-filter Lanczos")
mitchellReferenceOptions="-filter Mitchell"
allFilters="${filterKeys[*]}"

# --- Backgrounds behind transparent pixels (keys as --background=) -----------------------------------------------------

# ImageMagick takes the background color as numbers in the colorspace of the image, which is linear RGB at that point: the viewer gray is sRGB 128 = linear 21.586 %
declare -A backgroundReferenceColors=([black]="black" [gray]="srgb(21.586%,21.586%,21.586%)")

# A viewer that is started right after another one exited gets the same X window id, and KWin sometimes applied the late map and destroy
# of the old window to the new one, which killed it with an X error (BadWindow, GLXBadDrawable). --render-to keeps its window hidden now,
# so KWin never sees it; a render that still dies of an X error is simply repeated.
maximumRenderAttempts=3

# --- Test cases: image | scales | checks -------------------------------------------------------------------------------------
# A scale "1@1280x720" renders 1:1 into a window of that size (the setup of the screenshot measurements in plan section 1.2)
# Checks: flat188 (downscaled mean and std), ring (zone plate aliasing), gammaleft (left half of gamma_rows), colormix (red/green mix),
#         impulse (symmetry and sampled kernel), overshoot (step edge), exactnearest (Nearest at integer zoom), frame (all four frame sides),
#         nolevel0 (level 0 exceeds the GPU texture size, above 0.125 the PSNR is informational),
#         autonearest (rendered again with --nearest-from=autoNearestPercent: from that zoom on exactly Nearest, below it exactly the chosen filter)
# An optional fourth field sets the background behind transparent pixels (default black)

testCases=(
	"checker_1px|0.05 0.1 0.146 0.238 0.35 0.5 0.7 1 1@1280x720 1.5 2.3 4|flat188"
	"zoneplate_2048|0.05 0.1 0.146 0.238 0.35 0.5 0.7 1|ring"
	"zoneplate_4096|0.03 0.05 0.1 0.146 0.238 0.35|ring"
	"siemens_star|0.03 0.05 0.146 0.35 0.7|"
	"gamma_rows|0.05 0.146 0.238 0.35 0.5 0.7|gammaleft"
	"color_checker_1px|0.05 0.146 0.35 0.5 0.7|colormix"
	"impulse|1 1.5 2.3 4 7 8|impulse"
	"step_edge|0.35 0.7 1.5 2.3 4 8|overshoot"
	"lines_1px|0.03 0.05 0.146 0.238 0.35 0.5 0.7 1|"
	"text|0.03 0.05 0.5 0.7 1 1.5|"
	"pixelart_32|1 2 3 4 8|exactnearest autonearest"
	"alpha_disk|0.05 0.146 0.35 0.7 1 1.5|"
	"alpha_disk|0.05 0.146 0.35 1||gray"
	"border_frame_odd|0.03 0.05 0.146 0.238 0.35 0.7 1|frame"
	"extreme_aspect|0.02 0.5|nolevel0"
)
quickScales="0.05 0.146 0.35 1 1@1280x720 4 8"

# --- Arguments ---------------------------------------------------------------------------------------------------------------

viewer=""
onlyImages=""
selectedFilters="$allFilters"
isQuick=0
withPhoto=1
for argument in "$@"; do
	case "$argument" in
		--viewer=*) viewer="${argument#--viewer=}" ;;
		--images=*) onlyImages=",${argument#--images=}," ;;
		--filters=*) selectedFilters="${argument#--filters=}"; selectedFilters="${selectedFilters//,/ }" ;;
		--quick) isQuick=1 ;;
		--no-photo) withPhoto=0 ;;
		*) echo "Unknown argument '$argument'" >&2; exit 2 ;;
	esac
done
if [ -z "$viewer" ]; then
	for candidate in "$demosDirectory/build/FPL_ImageViewer/Linux-x64-Release/FPL_ImageViewer" "$demosDirectory/build/FPL_ImageViewer/Linux-x64-Debug/FPL_ImageViewer"; do
		if [ -x "$candidate" ]; then
			viewer="$candidate"
			break
		fi
	done
fi
if [ -z "$viewer" ] || [ ! -x "$viewer" ]; then
	echo "FPL_ImageViewer executable not found, build it or pass --viewer=<path>" >&2
	exit 2
fi
for filterKey in $selectedFilters; do
	if [ -z "${filterNames[$filterKey]+defined}" ]; then
		echo "Unknown filter '$filterKey', use: ${filterKeys[*]}" >&2
		exit 2
	fi
done
if [ ! -f "$imagesDirectory/checker_1px.png" ]; then
	echo "Test images missing in $imagesDirectory, run generate_testimages.sh" >&2
	exit 2
fi
mkdir -p "$rendersDirectory" "$referencesDirectory"

# --- Helpers -----------------------------------------------------------------------------------------------------------------

calc() {
	awk "BEGIN { printf \"%s\", $1 }"
}
calc_format() {
	awk "BEGIN { printf \"$1\", $2 }"
}
# True when a <= b (floats)
is_less_equal() {
	awk -v a="$1" -v b="$2" 'BEGIN { exit !(a <= b) }'
}
is_abs_less_equal() {
	awk -v a="$1" -v b="$2" 'BEGIN { d = a < 0 ? -a : a; exit !(d <= b) }'
}
round_to_int() {
	awk -v v="$1" 'BEGIN { r = int(v + 0.5); if (r < 1) { r = 1 }; printf "%d", r }'
}

# First number of a compare result, e.g. "41.2 (0.1)" -> 41.2
compare_value() {
	magick compare -metric "$1" "$2" "$3" null: 2>&1 | awk '{ print $1 }'
}
# Number of differing pixels; ImageMagick 7.1.2 reports AE as a channel weighted fraction, anything above zero counts as at least one pixel
compare_differing_pixels() {
	magick compare -metric AE "$1" "$2" null: 2>&1 | awk '{ v = $1 + 0; if (v > 0 && v < 1) { v = 1 }; printf "%.0f", v }'
}
# Mean red of the four one pixel wide sides: "top bottom left right"
frame_side_reds() {
	local image="$1" width="$2" height="$3"
	magick "$image" -channel R -separate +channel \( -clone 0 -crop "${width}x1+0+0" \) \( -clone 0 -crop "${width}x1+0+$(( height - 1 ))" \) \( -clone 0 -crop "1x${height}+0+0" \) \( -clone 0 -crop "1x${height}+$(( width - 1 ))+0" \) -delete 0 -format "%[fx:mean*255] " info:
}
# Normalized value of a compare result in 8 bit steps, e.g. PAE "514 (0.00784)" -> 2.0
compare_steps() {
	magick compare -metric "$1" "$2" "$3" null: 2>&1 | awk '{ gsub(/[()]/, "", $2); printf "%.1f", $2 * 255 }'
}
# Mean and standard deviation (8 bit steps) of the red channel inside a crop
crop_mean_std() {
	magick "$1" -channel R -separate +channel -crop "$2" +repage -format "%[fx:mean*255] %[fx:standard_deviation*255]" info:
}
# Mean of each channel inside a crop, "r g b"
crop_channel_means() {
	magick "$1" -crop "$2" +repage -format "%[fx:mean.r*255] %[fx:mean.g*255] %[fx:mean.b*255]" info:
}
# Standard deviation (8 bit steps) of the red channel inside the mask (white = inside)
masked_std() {
	magick "$1" -channel R -separate +channel "$2" \( -clone 0 -clone 1 -compose multiply -composite \) \( -clone 0 -clone 0 -compose multiply -composite -clone 1 -compose multiply -composite \) -delete 0 -format "%[fx:mean] " info: | awk '{ inside = $1; mean = $2 / inside; variance = $3 / inside - mean * mean; if (variance < 0) { variance = 0 }; printf "%.1f", sqrt(variance) * 255 }'
}

# Reference: resize in linear light with the given kernel, transparent pixels over the background in linear light as the viewer shows them
make_reference() {
	local source="$1" width="$2" height="$3" options="$4" target="$5" background="${6:-black}"
	if [ -f "$target" ] && [ "$target" -nt "$source" ]; then
		return
	fi
	# shellcheck disable=SC2086
	magick "$source" -colorspace sRGB -colorspace RGB $options -resize "${width}x${height}!" -background "${backgroundReferenceColors[$background]}" -alpha remove -alpha off -colorspace sRGB -depth 8 "$target"
}

# 1:1 reference: the source itself over the background
make_identity_reference() {
	local source="$1" target="$2" background="${3:-black}"
	if [ -f "$target" ] && [ "$target" -nt "$source" ]; then
		return
	fi
	magick "$source" -colorspace sRGB -colorspace RGB -background "${backgroundReferenceColors[$background]}" -alpha remove -alpha off -colorspace sRGB -depth 8 "$target"
}

# Renders one picture offscreen, repeats a render that died of an X error (see maximumRenderAttempts), returns the viewer exit code.
# Arguments after the level source are passed on to the viewer.
render_picture() {
	local target="$1" windowSize="$2" zoomParameter="$3" filterKey="$4" background="$5" source="$6" levelSource="${7:-auto}"
	local extraArguments=("${@:8}")
	local viewerOutput="$rendersDirectory/viewer_output.txt"
	local exitCode=0
	for (( attempt = 1; attempt <= maximumRenderAttempts; attempt++ )); do
		rm -f "$target"
		"$viewer" --render-to="$target" --window="$windowSize" "$zoomParameter" --down-filter="$filterKey" --up-filter="$filterKey" --background="$background" --lod-source="$levelSource" "${extraArguments[@]}" "$source" > "$viewerOutput" 2>&1
		exitCode=$?
		if [ "$exitCode" = 0 ] || ! grep -q "X Error" "$viewerOutput"; then
			break
		fi
		retriedRenderCount=$(( retriedRenderCount + 1 ))
	done
	return "$exitCode"
}

# --- Report --------------------------------------------------------------------------------------------------------------------

failedRowCount=0
rowCount=0
retriedRenderCount=0
reportRows=()

report_row() {
	local line="$1"
	reportRows+=("$line")
	echo "$line"
}

tableHeader="| Image | Scale | Output | Filter | PSNR (dB) | Checks | Result |"
tableSeparator="|---|---|---|---|---|---|---|"

echo "Viewer: $viewer"
echo "Results: $outputDirectory"
echo
echo "$tableHeader"
echo "$tableSeparator"

# --- Main loop -----------------------------------------------------------------------------------------------------------------

for testCase in "${testCases[@]}"; do
	IFS='|' read -r imageName scales checks background <<< "$testCase"
	background="${background:-black}"
	# Everything that depends on the background carries it in its file name, black is the plain name
	backgroundSuffix=""
	if [ "$background" != black ]; then
		backgroundSuffix="_$background"
	fi
	if [ -n "$onlyImages" ] && [[ "$onlyImages" != *",$imageName,"* ]]; then
		continue
	fi
	source="$imagesDirectory/$imageName.png"
	read -r sourceWidth sourceHeight isSourceOpaque <<< "$(magick identify -format "%w %h %[opaque]" "$source")"
	if [ "$isQuick" = 1 ]; then
		quickSelection=""
		for scale in $scales; do
			if [[ " $quickScales " == *" $scale "* ]]; then
				quickSelection+="$scale "
			fi
		done
		scales="$quickSelection"
	fi

	for scaleToken in $scales; do
		# Window size and zoom
		scale="${scaleToken%@*}"
		pictureWidth=$(round_to_int "$(calc "$sourceWidth * $scale")")
		pictureHeight=$(round_to_int "$(calc "$sourceHeight * $scale")")
		if [[ "$scaleToken" == *@* ]]; then
			windowSize="${scaleToken#*@}"
			windowWidth="${windowSize%x*}"
			windowHeight="${windowSize#*x}"
			zoomParameter="--zoom=$(calc "$scale * 100")"
			cropOffsetX=$(( (windowWidth - pictureWidth) / 2 ))
			cropOffsetY=$(( (windowHeight - pictureHeight) / 2 ))
		else
			windowWidth=$pictureWidth
			windowHeight=$pictureHeight
			# The exact scale and not fit: fit keeps the aspect ratio and can come out a pixel narrower than the rounded window (1023x767 at 0.35 fits as 357x268 into 358x268), the reference is resized to exactly the window
			zoomParameter="--zoom=$(calc "$scale * 100")"
			cropOffsetX=0
			cropOffsetY=0
		fi
		pictureCrop="${pictureWidth}x${pictureHeight}+${cropOffsetX}+${cropOffsetY}"
		caseKey="${imageName}_${scaleToken//@/_at_}${backgroundSuffix}"
		isIdentity=0
		if [ "$scale" = "1" ]; then
			isIdentity=1
		fi
		readsLevel=0
		if is_less_equal "$scale" "$largestLevelScale"; then
			readsLevel=1
		fi

		# References of all kernels for this size, in parallel
		if [ "$isIdentity" = 1 ]; then
			make_identity_reference "$source" "$referencesDirectory/${imageName}_identity${backgroundSuffix}.png" "$background"
		else
			for filterKey in $selectedFilters; do
				make_reference "$source" "$pictureWidth" "$pictureHeight" "${filterReferenceOptions[$filterKey]}" "$referencesDirectory/${imageName}_${pictureWidth}x${pictureHeight}_${filterKey}${backgroundSuffix}.png" "$background" &
			done
		fi
		if [[ " $checks " == *" ring "* ]]; then
			make_reference "$source" "$pictureWidth" "$pictureHeight" "$mitchellReferenceOptions" "$referencesDirectory/${imageName}_${pictureWidth}x${pictureHeight}_mitchell.png" &
		fi
		wait

		# Ring mask for the zone plate, shared by all filters of this size
		ringMask=""
		if [[ " $checks " == *" ring "* ]]; then
			ringCenterX=$(calc "($sourceWidth / 2 + 0.5) * $scale - 0.5")
			ringCenterY=$(calc "($sourceHeight / 2 + 0.5) * $scale - 0.5")
			outputNyquistRadius=$(calc "0.5 * $scale * $scale * $sourceWidth")
			ringInnerRadius=$(calc "$ringStartNyquistFactor * $outputNyquistRadius")
			ringOuterRadius=$(calc "$ringEndSourceNyquistFraction * 0.5 * $scale * $sourceWidth")
			if is_less_equal "$(calc "$ringInnerRadius + $ringMinimumWidth")" "$ringOuterRadius"; then
				ringMask="$referencesDirectory/${imageName}_${pictureWidth}x${pictureHeight}_ringmask.png"
				magick -size "${pictureWidth}x${pictureHeight}" xc:black +antialias -fill white -draw "circle $ringCenterX,$ringCenterY $(calc "$ringCenterX + $ringOuterRadius"),$ringCenterY" -fill black -draw "circle $ringCenterX,$ringCenterY $(calc "$ringCenterX + $ringInnerRadius"),$ringCenterY" "$ringMask"
				mitchellRing=$(masked_std "$referencesDirectory/${imageName}_${pictureWidth}x${pictureHeight}_mitchell.png" "$ringMask")
			fi
		fi

		for filterKey in $selectedFilters; do
			filterName="${filterNames[$filterKey]}"
			render="$rendersDirectory/${caseKey}_${filterKey}.pam"
			render_picture "$render" "${windowWidth}x${windowHeight}" "$zoomParameter" "$filterKey" "$background" "$source"
			viewerExitCode=$?
			rowCount=$(( rowCount + 1 ))
			outputLabel="${pictureWidth}×${pictureHeight}"
			if [ "$windowWidth" != "$pictureWidth" ] || [ "$windowHeight" != "$pictureHeight" ]; then
				outputLabel+=" in ${windowWidth}×${windowHeight}"
			fi
			if [ "$viewerExitCode" != 0 ] || [ ! -f "$render" ]; then
				report_row "| $imageName | $scaleToken | $outputLabel | $filterName | – | viewer exit code $viewerExitCode | **FAIL** |"
				failedRowCount=$(( failedRowCount + 1 ))
				continue
			fi

			# The picture area of the render
			picture="$render"
			if [ "$cropOffsetX" != 0 ] || [ "$cropOffsetY" != 0 ]; then
				picture="$rendersDirectory/${caseKey}_${filterKey}_picture.png"
				magick "$render" -crop "$pictureCrop" +repage "$picture"
			fi

			if [ "$isIdentity" = 1 ]; then
				reference="$referencesDirectory/${imageName}_identity${backgroundSuffix}.png"
			else
				reference="$referencesDirectory/${imageName}_${pictureWidth}x${pictureHeight}_${filterKey}${backgroundSuffix}.png"
			fi

			# Nearest is point sampling: it cannot average, and where a sample falls exactly between two source pixels the tie breaking is implementation defined
			isPointSampling=0
			isTieFreeScale=0
			if [ "$filterKey" = nearest ]; then
				isPointSampling=1
			fi
			if awk -v s="$scale" 'BEGIN { exit !(s >= 1 && s == int(s)) }'; then
				isTieFreeScale=1
			fi

			failures=""
			checkTexts=()
			psnr=$(compare_value PSNR "$picture" "$reference")
			if [ "$isPointSampling" = 1 ] && [ "$isTieFreeScale" = 0 ]; then
				checkTexts+=("PSNR informational (tie breaking)")
			elif [[ " $checks " == *" nolevel0 "* ]] && [ "$readsLevel" = 0 ]; then
				checkTexts+=("PSNR informational (level 0 exceeds the GPU, level 1 stands in)")
			elif ! is_less_equal "$psnrThreshold" "$psnr"; then
				failures+="psnr "
			fi

			# The same picture from level 0: the reduced level must not change what is shown
			levelPicture=""
			if [ "$readsLevel" = 1 ] && [[ " $checks " != *" nolevel0 "* ]]; then
				levelRender="$rendersDirectory/${caseKey}_${filterKey}_level0.pam"
				render_picture "$levelRender" "${windowWidth}x${windowHeight}" "$zoomParameter" "$filterKey" "$background" "$source" 0
				levelExitCode=$?
				if [ "$levelExitCode" != 0 ] || [ ! -f "$levelRender" ]; then
					checkTexts+=("level 0 render failed with exit code $levelExitCode")
					failures+="level0 "
				else
					levelPicture="$levelRender"
					if [ "$cropOffsetX" != 0 ] || [ "$cropOffsetY" != 0 ]; then
						levelPicture="$rendersDirectory/${caseKey}_${filterKey}_level0_picture.png"
						magick "$levelRender" -crop "$pictureCrop" +repage "$levelPicture"
					fi
					levelPsnr=$(compare_value PSNR "$picture" "$levelPicture")
					checkTexts+=("LOD vs level 0 $levelPsnr dB")
					if ! is_less_equal "$levelPsnrThreshold" "$levelPsnr"; then
						failures+="lod "
					fi
				fi
			fi

			if [ "$isIdentity" = 1 ]; then
				differentPixels=$(compare_differing_pixels "$picture" "$reference")
				if [ "$isSourceOpaque" = True ]; then
					checkTexts+=("1:1 differing pixels $differentPixels")
					if [ "$differentPixels" != 0 ]; then
						failures+="identity "
					fi
				else
					largestDifference=$(compare_steps PAE "$picture" "$reference")
					checkTexts+=("1:1 differing pixels $differentPixels, at most $largestDifference steps")
					if ! is_less_equal "$largestDifference" "$translucentIdentityTolerance"; then
						failures+="identity "
					fi
				fi
			fi

			if [[ " $checks " == *" flat188 "* ]] && is_less_equal "$scale" 0.99 && [ "$isPointSampling" = 0 ]; then
				centerCrop="$(( pictureWidth / 2 ))x$(( pictureHeight / 2 ))+$(( pictureWidth / 4 ))+$(( pictureHeight / 4 ))"
				read -r flatMean flatStd <<< "$(crop_mean_std "$picture" "$centerCrop")"
				flatText="flat $(calc_format "%.1f" "$flatMean") ± $(calc_format "%.1f" "$flatStd")"
				if ! is_abs_less_equal "$(calc "$flatMean - $linearHalfValue")" "$flatMeanTolerance" || ! is_less_equal "$flatStd" "$flatStdTolerance"; then
					read -r referenceMean referenceStd <<< "$(crop_mean_std "$reference" "$centerCrop")"
					flatText+=" (kernel property: reference $(calc_format "%.1f" "$referenceMean") ± $(calc_format "%.1f" "$referenceStd"))"
					if ! is_abs_less_equal "$(calc "$flatMean - $referenceMean")" "$kernelPropertyTolerance" || ! is_abs_less_equal "$(calc "$flatStd - $referenceStd")" "$kernelPropertyTolerance"; then
						failures+="flat "
					fi
				fi
				checkTexts+=("$flatText")
			elif [[ " $checks " == *" flat188 "* ]] && ! is_less_equal "$scale" 0.99; then
				read -r centerMean centerStd <<< "$(crop_mean_std "$picture" "$(( pictureWidth / 2 ))x$(( pictureHeight / 2 ))+$(( pictureWidth / 4 ))+$(( pictureHeight / 4 ))")"
				checkTexts+=("center $(calc_format "%.1f" "$centerMean") ± $(calc_format "%.1f" "$centerStd")")
			fi

			if [ -n "$ringMask" ]; then
				viewerRing=$(masked_std "$picture" "$ringMask")
				referenceRing=$(masked_std "$reference" "$ringMask")
				ringText="ring $viewerRing (same kernel $referenceRing, Mitchell $mitchellRing"
				if [ -n "$levelPicture" ]; then
					levelRing=$(masked_std "$levelPicture" "$ringMask")
					ringText+=", from level 0 $levelRing"
					if ! is_abs_less_equal "$(calc "$viewerRing - $levelRing")" "$levelRingTolerance"; then
						failures+="lodring "
					fi
				fi
				checkTexts+=("$ringText)")
				if ! is_less_equal "$viewerRing" "$(calc "$referenceRing + $ringTolerance")"; then
					failures+="ring "
				fi
			fi

			if [[ " $checks " == *" gammaleft "* ]] && [ "$isPointSampling" = 0 ]; then
				leftCrop="$(( pictureWidth * 3 / 10 ))x$(( pictureHeight * 8 / 10 ))+$(( pictureWidth / 10 ))+$(( pictureHeight / 10 ))"
				read -r leftMean leftStd <<< "$(crop_mean_std "$picture" "$leftCrop")"
				gammaText="left half $(calc_format "%.1f" "$leftMean") ± $(calc_format "%.1f" "$leftStd")"
				if ! is_abs_less_equal "$(calc "$leftMean - $linearHalfValue")" "$gammaTolerance"; then
					read -r referenceMean referenceStd <<< "$(crop_mean_std "$reference" "$leftCrop")"
					gammaText+=" (kernel property: reference $(calc_format "%.1f" "$referenceMean") ± $(calc_format "%.1f" "$referenceStd"))"
					if ! is_abs_less_equal "$(calc "$leftMean - $referenceMean")" "$kernelPropertyTolerance"; then
						failures+="gamma "
					fi
				fi
				checkTexts+=("$gammaText")
			fi

			if [[ " $checks " == *" colormix "* ]] && [ "$isPointSampling" = 0 ]; then
				centerCrop="$(( pictureWidth / 2 ))x$(( pictureHeight / 2 ))+$(( pictureWidth / 4 ))+$(( pictureHeight / 4 ))"
				read -r mixRed mixGreen mixBlue <<< "$(crop_channel_means "$picture" "$centerCrop")"
				mixText="mix ($(calc_format "%.0f" "$mixRed"), $(calc_format "%.0f" "$mixGreen"), $(calc_format "%.0f" "$mixBlue"))"
				if ! is_abs_less_equal "$(calc "$mixRed - $linearHalfValue")" "$gammaTolerance" || ! is_abs_less_equal "$(calc "$mixGreen - $linearHalfValue")" "$gammaTolerance"; then
					read -r referenceRed referenceGreen referenceBlue <<< "$(crop_channel_means "$reference" "$centerCrop")"
					mixText+=" (kernel property: reference ($(calc_format "%.0f" "$referenceRed"), $(calc_format "%.0f" "$referenceGreen"), $(calc_format "%.0f" "$referenceBlue")))"
					if ! is_abs_less_equal "$(calc "$mixRed - $referenceRed")" "$kernelPropertyTolerance" || ! is_abs_less_equal "$(calc "$mixGreen - $referenceGreen")" "$kernelPropertyTolerance"; then
						failures+="mix "
					fi
				fi
				checkTexts+=("$mixText")
			fi

			if [[ " $checks " == *" impulse "* ]]; then
				flopped="$rendersDirectory/${caseKey}_${filterKey}_flop.png"
				magick "$picture" -flop "$flopped"
				asymmetry=$(compare_steps PAE "$picture" "$flopped")
				kernelDeviation=$(compare_steps PAE "$picture" "$reference")
				checkTexts+=("asymmetry $asymmetry, max deviation from kernel $kernelDeviation")
				if ! is_less_equal "$asymmetry" "$impulseTolerance" || ! is_less_equal "$kernelDeviation" "$impulseTolerance"; then
					failures+="impulse "
				fi
			fi

			if [[ " $checks " == *" overshoot "* ]]; then
				read -r viewerMinimum viewerMaximum <<< "$(magick "$picture" -channel R -separate -format "%[fx:minima*255] %[fx:maxima*255]" info:)"
				read -r referenceMinimum referenceMaximum <<< "$(magick "$reference" -channel R -separate -format "%[fx:minima*255] %[fx:maxima*255]" info:)"
				checkTexts+=("range $(calc_format "%.0f" "$viewerMinimum")–$(calc_format "%.0f" "$viewerMaximum") (reference $(calc_format "%.0f" "$referenceMinimum")–$(calc_format "%.0f" "$referenceMaximum"), source 64–191)")
			fi

			if [[ " $checks " == *" exactnearest "* ]] && [ "$filterKey" = nearest ]; then
				differentPixels=$(compare_differing_pixels "$picture" "$reference")
				checkTexts+=("differing pixels $differentPixels")
				if [ "$differentPixels" != 0 ]; then
					failures+="nearest "
				fi
			fi

			if [[ " $checks " == *" autonearest "* ]]; then
				autoRender="$rendersDirectory/${caseKey}_${filterKey}_autonearest.pam"
				render_picture "$autoRender" "${windowWidth}x${windowHeight}" "$zoomParameter" "$filterKey" "$background" "$source" auto "--nearest-from=$autoNearestPercent"
				autoExitCode=$?
				if [ "$autoExitCode" != 0 ] || [ ! -f "$autoRender" ]; then
					checkTexts+=("auto Nearest render failed with exit code $autoExitCode")
					failures+="autonearest "
				elif awk -v s="$scale" -v p="$autoNearestPercent" 'BEGIN { exit !(s * 100 >= p) }'; then
					nearestReference="$referencesDirectory/${imageName}_${pictureWidth}x${pictureHeight}_nearest${backgroundSuffix}.png"
					make_reference "$source" "$pictureWidth" "$pictureHeight" "${filterReferenceOptions[nearest]}" "$nearestReference" "$background"
					autoDifferentPixels=$(compare_differing_pixels "$autoRender" "$nearestReference")
					checkTexts+=("from $autoNearestPercent % Nearest: differing pixels $autoDifferentPixels")
					if [ "$autoDifferentPixels" != 0 ]; then
						failures+="autonearest "
					fi
				else
					autoDifferentPixels=$(compare_differing_pixels "$autoRender" "$render")
					checkTexts+=("below $autoNearestPercent % unchanged: differing pixels $autoDifferentPixels")
					if [ "$autoDifferentPixels" != 0 ]; then
						failures+="autonearest "
					fi
				fi
			fi

			if [[ " $checks " == *" frame "* ]]; then
				read -r -a viewerSides <<< "$(frame_side_reds "$picture" "$pictureWidth" "$pictureHeight")"
				read -r -a referenceSides <<< "$(frame_side_reds "$reference" "$pictureWidth" "$pictureHeight")"
				sideLabels=(top bottom left right)
				sideTexts=""
				isFrameCut=0
				for sideIndex in 0 1 2 3; do
					sideTexts+="${sideLabels[$sideIndex]} $(calc_format "%.0f" "${viewerSides[$sideIndex]}")/$(calc_format "%.0f" "${referenceSides[$sideIndex]}") "
					if ! is_less_equal "$(calc "$frameMinimumFraction * ${referenceSides[$sideIndex]}")" "${viewerSides[$sideIndex]}"; then
						isFrameCut=1
					fi
				done
				checkTexts+=("frame red (viewer/reference) ${sideTexts% }")
				if [ "$isFrameCut" = 1 ]; then
					failures+="frame "
				fi
			fi

			checkText=""
			for text in "${checkTexts[@]+"${checkTexts[@]}"}"; do
				if [ -n "$checkText" ]; then
					checkText+="; "
				fi
				checkText+="$text"
			done
			if [ -z "$failures" ]; then
				result="ok"
			else
				result="**FAIL** (${failures% })"
				failedRowCount=$(( failedRowCount + 1 ))
			fi
			imageLabel="$imageName"
			if [ "$background" != black ]; then
				imageLabel+=" on $background"
			fi
			report_row "| $imageLabel | $scaleToken | $outputLabel | $filterName | $psnr | $checkText | $result |"
		done
	done
done

# --- Real photo: filters barely differ, informational only ------------------------------------------------------------------

# 960x720 is the photo fitted into 1280x720 (level 0), 200x150 reads level 1; both keep the 4:3 aspect ratio exactly
photoRows=()
photoSizes="960x720 200x150"
if [ "$withPhoto" = 1 ] && [ -f "$photoFile" ] && [ -z "$onlyImages" ]; then
	for photoSize in $photoSizes; do
		photoWindowWidth="${photoSize%x*}"
		photoWindowHeight="${photoSize#*x}"
		photoReference="$referencesDirectory/photo_${photoWindowWidth}x${photoWindowHeight}_mitchell.png"
		make_reference "$photoFile" "$photoWindowWidth" "$photoWindowHeight" "$mitchellReferenceOptions" "$photoReference"
		for filterKey in $selectedFilters; do
			render="$rendersDirectory/photo_${photoSize}_${filterKey}.pam"
			levelRender="$rendersDirectory/photo_${photoSize}_${filterKey}_level0.pam"
			render_picture "$render" "${photoWindowWidth}x${photoWindowHeight}" --zoom=fit "$filterKey" black "$photoFile"
			render_picture "$levelRender" "${photoWindowWidth}x${photoWindowHeight}" --zoom=fit "$filterKey" black "$photoFile" 0
			photoPsnr=$(compare_value PSNR "$render" "$photoReference")
			photoLevelPsnr=$(compare_value PSNR "$render" "$levelRender")
			photoRows+=("| $(basename "$photoFile") | ${photoWindowWidth}×${photoWindowHeight} | ${filterNames[$filterKey]} | $photoPsnr | $photoLevelPsnr |")
		done
	done
fi

# --- Write report ------------------------------------------------------------------------------------------------------------

{
	echo "# FPL_ImageViewer scaling tests"
	echo
	echo "Viewer: \`$viewer\`  "
	echo "Date: $(date '+%Y-%m-%d %H:%M')  "
	echo "Rows: $rowCount, failed: $failedRowCount, renders repeated after an X error: $retriedRenderCount"
	echo
	echo "$tableHeader"
	echo "$tableSeparator"
	for line in "${reportRows[@]+"${reportRows[@]}"}"; do
		echo "$line"
	done
	if [ "${#photoRows[@]}" -gt 0 ]; then
		echo
		echo "## Photo against the Mitchell reference (informational)"
		echo
		echo "| Photo | Output | Filter | PSNR (dB) | LOD vs level 0 (dB) |"
		echo "|---|---|---|---|---|"
		for line in "${photoRows[@]}"; do
			echo "$line"
		done
	fi
} > "$reportFile"

echo
if [ "${#photoRows[@]}" -gt 0 ]; then
	echo "Photo against the Mitchell reference:"
	for line in "${photoRows[@]}"; do
		echo "$line"
	done
	echo
fi
echo "Rows: $rowCount, failed: $failedRowCount, renders repeated after an X error: $retriedRenderCount"
echo "Report: $reportFile"
if [ "$failedRowCount" -gt 0 ]; then
	exit 1
fi
exit 0
