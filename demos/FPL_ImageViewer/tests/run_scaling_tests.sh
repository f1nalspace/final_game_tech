#!/bin/bash
# Renders the synthetic test images with FPL_ImageViewer --render-to at many scales and with every filter,
# compares each result against an ImageMagick reference of the same kernel computed in linear light and prints a table.
# Exit code is 0 when every threshold holds, 1 when at least one is violated, 2 on usage or setup errors.
#
# Usage: run_scaling_tests.sh [--viewer=<path>] [--images=<name,...>] [--filters=<number,...>] [--quick] [--no-photo]
#   --viewer   FPL_ImageViewer executable (default: Release build, then Debug build under demos/build/FPL_ImageViewer)
#   --images   only these test images, names without extension (e.g. zoneplate_2048,checker_1px)
#   --filters  only these viewer filters (1 = Nearest ... 7 = Lanczos3, same numbers as -f=)
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
# Minimum mean red along each side of border_frame_odd, as a fraction of the same side in the reference; a cut off frame side stays far below
frameMinimumFraction=0.5
# Expected sRGB value of a 50 % linear mix of black and white (and of a 50 % red / green mix per channel)
linearHalfValue=188

# --- Ring metric for the zone plate --------------------------------------------------------------------------------------

# The ring starts at this multiple of the output Nyquist radius (a correct filter is flat there) ...
ringStartNyquistFactor=1.5
# ... and ends at this fraction of the radius where the source itself reaches Nyquist
ringEndSourceNyquistFraction=0.95
# Rings thinner than this (output pixels) are not measured
ringMinimumWidth=4

# --- Viewer filters (numbers as -f=) and their ImageMagick equivalents -----------------------------------------------------

filterNames=("" "Nearest" "Bilinear" "Bicubic (Triangular)" "Bicubic (Bell)" "Bicubic (B-Spline)" "Catmull-Rom" "Lanczos3")
filterKeys=("" "nearest" "bilinear" "triangular" "bell" "bspline" "catmullrom" "lanczos3")
# Triangular is a triangle of radius 2 and Bell a quadratic B-spline stretched to radius 2, blur widens the ImageMagick kernel accordingly
filterReferenceOptions=("" "-filter Point" "-filter Triangle" "-filter Triangle -define filter:blur=2" "-filter Quadratic -define filter:blur=1.3333333333" "-filter Spline" "-filter Catrom" "-filter Lanczos")
mitchellReferenceOptions="-filter Mitchell"
allFilters="1 2 3 4 5 6 7"

# --- Test cases: image | scales | checks -------------------------------------------------------------------------------------
# A scale "1@1280x720" renders 1:1 into a window of that size (the setup of the screenshot measurements in plan section 1.2)
# Checks: flat188 (downscaled mean and std), ring (zone plate aliasing), gammaleft (left half of gamma_rows), colormix (red/green mix),
#         impulse (symmetry and sampled kernel), overshoot (step edge), exactnearest (Nearest at integer zoom), frame (all four frame sides)

testCases=(
	"checker_1px|0.1 0.146 0.238 0.35 0.5 0.7 1 1@1280x720 1.5 2.3 4|flat188"
	"zoneplate_2048|0.1 0.146 0.238 0.35 0.5 0.7 1|ring"
	"zoneplate_4096|0.1 0.146 0.238 0.35|ring"
	"siemens_star|0.146 0.35 0.7|"
	"gamma_rows|0.146 0.238 0.35 0.5 0.7|gammaleft"
	"color_checker_1px|0.146 0.35 0.5 0.7|colormix"
	"impulse|1 1.5 2.3 4 7 8|impulse"
	"step_edge|0.35 0.7 1.5 2.3 4 8|overshoot"
	"lines_1px|0.146 0.238 0.35 0.5 0.7 1|"
	"text|0.5 0.7 1 1.5|"
	"pixelart_32|1 2 3 4 8|exactnearest"
	"alpha_disk|0.146 0.35 0.7 1 1.5|"
	"border_frame_odd|0.35 0.7 1|frame"
)
quickScales="0.146 0.35 1 1@1280x720 4 8"

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

# Reference: resize in linear light with the given kernel, transparent pixels over black as the viewer shows them
make_reference() {
	local source="$1" width="$2" height="$3" options="$4" target="$5"
	if [ -f "$target" ] && [ "$target" -nt "$source" ]; then
		return
	fi
	# shellcheck disable=SC2086
	magick "$source" -colorspace sRGB -colorspace RGB $options -resize "${width}x${height}!" -background black -alpha remove -alpha off -colorspace sRGB -depth 8 "$target"
}

# 1:1 reference: the source itself over black
make_identity_reference() {
	local source="$1" target="$2"
	if [ -f "$target" ] && [ "$target" -nt "$source" ]; then
		return
	fi
	magick "$source" -colorspace sRGB -colorspace RGB -background black -alpha remove -alpha off -colorspace sRGB -depth 8 "$target"
}

# --- Report --------------------------------------------------------------------------------------------------------------------

failedRowCount=0
rowCount=0
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
	IFS='|' read -r imageName scales checks <<< "$testCase"
	if [ -n "$onlyImages" ] && [[ "$onlyImages" != *",$imageName,"* ]]; then
		continue
	fi
	source="$imagesDirectory/$imageName.png"
	read -r sourceWidth sourceHeight <<< "$(magick identify -format "%w %h" "$source")"
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
		caseKey="${imageName}_${scaleToken//@/_at_}"
		isIdentity=0
		if [ "$scale" = "1" ]; then
			isIdentity=1
		fi

		# References of all kernels for this size, in parallel
		if [ "$isIdentity" = 1 ]; then
			make_identity_reference "$source" "$referencesDirectory/${imageName}_identity.png"
		else
			for filterNumber in $selectedFilters; do
				make_reference "$source" "$pictureWidth" "$pictureHeight" "${filterReferenceOptions[$filterNumber]}" "$referencesDirectory/${imageName}_${pictureWidth}x${pictureHeight}_${filterKeys[$filterNumber]}.png" &
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

		for filterNumber in $selectedFilters; do
			filterName="${filterNames[$filterNumber]}"
			render="$rendersDirectory/${caseKey}_f${filterNumber}.pam"
			rm -f "$render"
			"$viewer" --render-to="$render" --window="${windowWidth}x${windowHeight}" "$zoomParameter" -f="$filterNumber" "$source" > /dev/null 2>&1
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
				picture="$rendersDirectory/${caseKey}_f${filterNumber}_picture.png"
				magick "$render" -crop "$pictureCrop" +repage "$picture"
			fi

			if [ "$isIdentity" = 1 ]; then
				reference="$referencesDirectory/${imageName}_identity.png"
			else
				reference="$referencesDirectory/${imageName}_${pictureWidth}x${pictureHeight}_${filterKeys[$filterNumber]}.png"
			fi

			# Nearest is point sampling: it cannot average, and where a sample falls exactly between two source pixels the tie breaking is implementation defined
			isPointSampling=0
			isTieFreeScale=0
			if [ "$filterNumber" = 1 ]; then
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
			elif ! is_less_equal "$psnrThreshold" "$psnr"; then
				failures+="psnr "
			fi

			if [ "$isIdentity" = 1 ]; then
				differentPixels=$(compare_differing_pixels "$picture" "$reference")
				checkTexts+=("1:1 differing pixels $differentPixels")
				if [ "$differentPixels" != 0 ]; then
					failures+="identity "
				fi
			fi

			if [[ " $checks " == *" flat188 "* ]] && is_less_equal "$scale" 0.99 && [ "$isPointSampling" = 0 ]; then
				centerCrop="$(( pictureWidth / 2 ))x$(( pictureHeight / 2 ))+$(( pictureWidth / 4 ))+$(( pictureHeight / 4 ))"
				read -r flatMean flatStd <<< "$(crop_mean_std "$picture" "$centerCrop")"
				checkTexts+=("flat $(calc_format "%.1f" "$flatMean") ± $(calc_format "%.1f" "$flatStd")")
				if ! is_abs_less_equal "$(calc "$flatMean - $linearHalfValue")" "$flatMeanTolerance" || ! is_less_equal "$flatStd" "$flatStdTolerance"; then
					failures+="flat "
				fi
			elif [[ " $checks " == *" flat188 "* ]] && ! is_less_equal "$scale" 0.99; then
				read -r centerMean centerStd <<< "$(crop_mean_std "$picture" "$(( pictureWidth / 2 ))x$(( pictureHeight / 2 ))+$(( pictureWidth / 4 ))+$(( pictureHeight / 4 ))")"
				checkTexts+=("center $(calc_format "%.1f" "$centerMean") ± $(calc_format "%.1f" "$centerStd")")
			fi

			if [ -n "$ringMask" ]; then
				viewerRing=$(masked_std "$picture" "$ringMask")
				referenceRing=$(masked_std "$reference" "$ringMask")
				checkTexts+=("ring $viewerRing (same kernel $referenceRing, Mitchell $mitchellRing)")
				if ! is_less_equal "$viewerRing" "$(calc "$referenceRing + $ringTolerance")"; then
					failures+="ring "
				fi
			fi

			if [[ " $checks " == *" gammaleft "* ]] && [ "$isPointSampling" = 0 ]; then
				leftCrop="$(( pictureWidth * 3 / 10 ))x$(( pictureHeight * 8 / 10 ))+$(( pictureWidth / 10 ))+$(( pictureHeight / 10 ))"
				read -r leftMean leftStd <<< "$(crop_mean_std "$picture" "$leftCrop")"
				checkTexts+=("left half $(calc_format "%.1f" "$leftMean") ± $(calc_format "%.1f" "$leftStd")")
				if ! is_abs_less_equal "$(calc "$leftMean - $linearHalfValue")" "$gammaTolerance"; then
					failures+="gamma "
				fi
			fi

			if [[ " $checks " == *" colormix "* ]] && [ "$isPointSampling" = 0 ]; then
				centerCrop="$(( pictureWidth / 2 ))x$(( pictureHeight / 2 ))+$(( pictureWidth / 4 ))+$(( pictureHeight / 4 ))"
				read -r mixRed mixGreen mixBlue <<< "$(crop_channel_means "$picture" "$centerCrop")"
				checkTexts+=("mix ($(calc_format "%.0f" "$mixRed"), $(calc_format "%.0f" "$mixGreen"), $(calc_format "%.0f" "$mixBlue"))")
				if ! is_abs_less_equal "$(calc "$mixRed - $linearHalfValue")" "$gammaTolerance" || ! is_abs_less_equal "$(calc "$mixGreen - $linearHalfValue")" "$gammaTolerance"; then
					failures+="mix "
				fi
			fi

			if [[ " $checks " == *" impulse "* ]]; then
				flopped="$rendersDirectory/${caseKey}_f${filterNumber}_flop.png"
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

			if [[ " $checks " == *" exactnearest "* ]] && [ "$filterNumber" = 1 ]; then
				differentPixels=$(compare_differing_pixels "$picture" "$reference")
				checkTexts+=("differing pixels $differentPixels")
				if [ "$differentPixels" != 0 ]; then
					failures+="nearest "
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
			report_row "| $imageName | $scaleToken | $outputLabel | $filterName | $psnr | $checkText | $result |"
		done
	done
done

# --- Real photo: filters barely differ, informational only ------------------------------------------------------------------

photoRows=()
if [ "$withPhoto" = 1 ] && [ -f "$photoFile" ] && [ -z "$onlyImages" ]; then
	photoWindowWidth=960
	photoWindowHeight=720
	photoReference="$referencesDirectory/photo_${photoWindowWidth}x${photoWindowHeight}_mitchell.png"
	make_reference "$photoFile" "$photoWindowWidth" "$photoWindowHeight" "$mitchellReferenceOptions" "$photoReference"
	for filterNumber in $selectedFilters; do
		render="$rendersDirectory/photo_f${filterNumber}.pam"
		"$viewer" --render-to="$render" --window="${photoWindowWidth}x${photoWindowHeight}" --zoom=fit -f="$filterNumber" "$photoFile" > /dev/null 2>&1
		photoPsnr=$(compare_value PSNR "$render" "$photoReference")
		photoRows+=("| $(basename "$photoFile") | ${photoWindowWidth}×${photoWindowHeight} | ${filterNames[$filterNumber]} | $photoPsnr |")
	done
fi

# --- Write report ------------------------------------------------------------------------------------------------------------

{
	echo "# FPL_ImageViewer scaling tests"
	echo
	echo "Viewer: \`$viewer\`  "
	echo "Date: $(date '+%Y-%m-%d %H:%M')  "
	echo "Rows: $rowCount, failed: $failedRowCount"
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
		echo "| Photo | Output | Filter | PSNR (dB) |"
		echo "|---|---|---|---|"
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
echo "Rows: $rowCount, failed: $failedRowCount"
echo "Report: $reportFile"
if [ "$failedRowCount" -gt 0 ]; then
	exit 1
fi
exit 0
