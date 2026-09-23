#!/bin/bash
# Generates the synthetic test images for the FPL_ImageViewer scaling, loader and orientation tests.
# The images are checked in under tests/images, this script documents how they were made and rebuilds them.
# Requires ImageMagick 7 (magick) with the DejaVu Sans font, output is deterministic (no timestamps, no random numbers).
#
# Usage: generate_testimages.sh [output directory]   (default: images next to this script)

set -euo pipefail

scriptDirectory="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
outputDirectory="${1:-$scriptDirectory/images}"
mkdir -p "$outputDirectory"
cd "$outputDirectory"

# No timestamps and no metadata, so regenerating gives byte identical files
pngOptions=(-strip -define png:exclude-chunks=date,time)
# ImageMagick reduces PNGs to palette or 1 bit gray on its own, these force the stored type (PNG24: prefix = 8 bit RGB)
pngGray8Options=(-define png:color-type=0 -define png:bit-depth=8)
# Nearly lossless JPEGs without chroma subsampling, so rotated variants stay comparable
jpegOptions=(-strip -quality 98 -sampling-factor 1x1)

step() {
	echo "  $1"
}

# Writes a gray image from an -fx expression
fx_gray() {
	local width="$1" height="$2" expression="$3" target="$4"
	magick -size "${width}x${height}" xc: -fx "$expression" -type Grayscale -depth 8 "${pngOptions[@]}" "${pngGray8Options[@]}" "$target"
}

# Writes an RGB image from three -fx expressions, one per channel
fx_rgb() {
	local width="$1" height="$2" red="$3" green="$4" blue="$5" target="$6"
	magick \( -size "${width}x${height}" xc: -fx "$red" \) \( -size "${width}x${height}" xc: -fx "$green" \) \( -size "${width}x${height}" xc: -fx "$blue" \) -set colorspace sRGB -combine -type TrueColor -depth 8 "${pngOptions[@]}" "PNG24:$target"
}

# Little endian integers as raw bytes, for hand made BMP and EXIF headers
le16() {
	local value=$(( $1 & 0xFFFF ))
	printf "\\x$(printf %02x $(( value & 0xFF )))\\x$(printf %02x $(( (value >> 8) & 0xFF )))"
}
le32() {
	local value=$(( $1 & 0xFFFFFFFF ))
	le16 $(( value & 0xFFFF ))
	le16 $(( (value >> 16) & 0xFFFF ))
}

# Inserts an APP1 Exif segment right after SOI; tiffBytes is the printf escaped TIFF block (header + IFD0)
insert_exif() {
	local source="$1" tiffBytes="$2" target="$3"
	local exifSignatureLength=6
	local segmentLengthFieldSize=2
	local tiffLength
	tiffLength=$(printf "$tiffBytes" | wc -c)
	local segmentLength=$(( segmentLengthFieldSize + exifSignatureLength + tiffLength ))
	{
		head -c 2 "$source"
		printf '\xff\xe1'
		printf "\\x$(printf %02x $(( segmentLength >> 8 )))\\x$(printf %02x $(( segmentLength & 0xFF )))"
		printf 'Exif\x00\x00'
		printf "$tiffBytes"
		tail -c +3 "$source"
	} > "$target"
}

# TIFF block with a single IFD0 entry: orientation tag 0x0112, type SHORT, count 1; byteOrder is MM (big endian) or II (little endian)
exif_orientation_block() {
	local orientation="$1" byteOrder="$2"
	if [ "$byteOrder" = "MM" ]; then
		printf '%s' "MM\\x00\\x2a\\x00\\x00\\x00\\x08\\x00\\x01\\x01\\x12\\x00\\x03\\x00\\x00\\x00\\x01\\x00\\x0${orientation}\\x00\\x00\\x00\\x00\\x00\\x00"
	else
		printf '%s' "II\\x2a\\x00\\x08\\x00\\x00\\x00\\x01\\x00\\x12\\x01\\x03\\x00\\x01\\x00\\x00\\x00\\x0${orientation}\\x00\\x00\\x00\\x00\\x00\\x00\\x00"
	fi
}

echo "Generating test images into $outputDirectory"

# --- Scaling: 1:1 fidelity and averaging in linear light ------------------------------------------------------------------

checkerSize=512
step "checker_1px.png"
fx_rgb $checkerSize $checkerSize "(i+j)%2" "(i+j)%2" "(i+j)%2" checker_1px.png

step "color_checker_1px.png"
fx_rgb $checkerSize $checkerSize "(i+j)%2" "1-(i+j)%2" "0" color_checker_1px.png

# Left half: alternating black and white rows (linear mean 0.5 = sRGB 188), right half: flat 188 with a strip of 128 (the wrong, gamma space mean)
gammaSize=1024
step "gamma_rows.png"
fx_gray $gammaSize $gammaSize "i < w/2 ? j%2 : ((j >= h*3/8 && j < h*5/8) ? 128/255 : 188/255)" gamma_rows.png

# --- Scaling: aliasing above the output Nyquist limit ------------------------------------------------------------------------

# Local frequency is r/w cycles per pixel, reaching the source Nyquist limit (0.5) at r = w/2
for zonePlateSize in 2048 4096; do
	step "zoneplate_${zonePlateSize}.png"
	fx_gray $zonePlateSize $zonePlateSize "0.5+0.5*cos(pi*((i-w/2)^2+(j-h/2)^2)/w)" "zoneplate_${zonePlateSize}.png"
done

# Sinusoidal star, the center where the spokes exceed the source Nyquist limit is flat gray
siemensSize=2048
siemensSpokeCount=72
siemensFlatCenterRadius=32
step "siemens_star.png"
fx_gray $siemensSize $siemensSize "hypot(i-w/2,j-h/2) < $siemensFlatCenterRadius ? 0.5 : 0.5+0.5*cos($siemensSpokeCount*atan2(j-h/2,i-w/2))" siemens_star.png

# Four quadrants: 0 degree lines every 4 px, 7 degree lines every 8 px, 45 degree lines every 6 px, vertical line pairs with 1-8 px gap
linesQuadrantSize=1024
linesQuadrantLast=$(( linesQuadrantSize - 1 ))
horizontalSpacing=4
sevenDegreeSpacing=8
sevenDegreeRise=126
diagonalSpacing=6
pairBandHeight=$(( linesQuadrantSize / 8 ))
pairPeriod=24
step "lines_1px.png"
drawHorizontal=""
for (( y = 0; y < linesQuadrantSize; y += horizontalSpacing )); do
	drawHorizontal+="line 0,$y $linesQuadrantLast,$y "
done
drawSevenDegree=""
for (( y = -sevenDegreeRise; y < linesQuadrantSize; y += sevenDegreeSpacing )); do
	drawSevenDegree+="line 0,$y $linesQuadrantLast,$(( y + sevenDegreeRise )) "
done
drawDiagonal=""
for (( x = -linesQuadrantLast; x < linesQuadrantSize; x += diagonalSpacing )); do
	drawDiagonal+="line $x,0 $(( x + linesQuadrantLast )),$linesQuadrantLast "
done
drawPairs=""
for (( gap = 1; gap <= 8; ++gap )); do
	bandTop=$(( (gap - 1) * pairBandHeight ))
	bandBottom=$(( bandTop + pairBandHeight - 2 ))
	for (( x = 4; x + gap + 1 < linesQuadrantSize; x += pairPeriod )); do
		drawPairs+="line $x,$bandTop $x,$bandBottom line $(( x + gap + 1 )),$bandTop $(( x + gap + 1 )),$bandBottom "
	done
done
lineOptions=(-size "${linesQuadrantSize}x${linesQuadrantSize}" xc:white -stroke black -strokewidth 1 +antialias)
magick \( "${lineOptions[@]}" -draw "$drawHorizontal" \) \( "${lineOptions[@]}" -draw "$drawSevenDegree" \) +append \( \( "${lineOptions[@]}" -draw "$drawDiagonal" \) \( "${lineOptions[@]}" -draw "$drawPairs" \) +append \) -append -type Grayscale -depth 8 "${pngOptions[@]}" "${pngGray8Options[@]}" lines_1px.png

# --- Scaling: kernel shape, overshoot, text, pixel art, alpha, borders ----------------------------------------------------

impulseSize=33
impulseCenter=$(( impulseSize / 2 ))
step "impulse.png"
magick -size "${impulseSize}x${impulseSize}" xc:black -fill white -draw "point $impulseCenter,$impulseCenter" -type TrueColor -depth 8 "${pngOptions[@]}" PNG24:impulse.png

# Levels 64 and 191 leave room for over- and undershoot in 8 bit
stepEdgeSize=256
step "step_edge.png"
fx_rgb $stepEdgeSize $stepEdgeSize "i < w/2 ? 0.25 : 0.75" "i < w/2 ? 0.25 : 0.75" "i < w/2 ? 0.25 : 0.75" step_edge.png

# Dark text on light and light text on dark, 9-24 px (density 72 -> 1 pt = 1 px)
textWidth=1200
textHalfHeight=400
textMargin=16
textLineSpacingPercent=150
textSample="The quick brown fox jumps over the lazy dog 0123456789 ÄÖÜ äöüß (){}[] /\\|"
step "text.png"
buildTextDraw() {
	local color="$1"
	local arguments=(-fill "$color")
	local y=$textMargin
	for fontSize in 9 10 11 12 14 16 18 20 24; do
		y=$(( y + fontSize * textLineSpacingPercent / 100 ))
		arguments+=(-pointsize "$fontSize" -annotate "+${textMargin}+${y}" "$fontSize px  $textSample")
	done
	printf '%s\0' "${arguments[@]}"
}
mapfile -d '' darkOnLight < <(buildTextDraw black)
mapfile -d '' lightOnDark < <(buildTextDraw white)
magick -density 72 -font DejaVu-Sans \( -size "${textWidth}x${textHalfHeight}" xc:white "${darkOnLight[@]}" \) \( -size "${textWidth}x${textHalfHeight}" xc:black "${lightOnDark[@]}" \) -append -type TrueColor -depth 8 "${pngOptions[@]}" PNG24:text.png

# Pseudo random flat colors (4 levels per channel) from a hash, no random number generator involved
pixelArtSize=32
pixelArtLevels=4
hashExpression() {
	local hash="sin(i*12.9898+j*78.233+$1)*43758.5453"
	echo "floor($pixelArtLevels*($hash-floor($hash)))/($pixelArtLevels-1)"
}
step "pixelart_32.png"
fx_rgb $pixelArtSize $pixelArtSize "$(hashExpression 0)" "$(hashExpression 17.17)" "$(hashExpression 43.43)" pixelart_32.png

# Opaque inner disk, half transparent ring, anti-aliased edges; fully transparent pixels carry lime RGB to expose premultiplication errors
alphaSize=1024
alphaInnerRadius=250
alphaOuterRadius=400
step "alpha_disk.png"
radiusExpression="hypot(i-w/2+0.5,j-h/2+0.5)"
alphaExpression="clamp($alphaOuterRadius+0.5-$radiusExpression) * (0.5 + 0.5*clamp($alphaInnerRadius+0.5-$radiusExpression))"
outsideExpression="$radiusExpression >= $alphaOuterRadius+0.5"
magick \( -size "${alphaSize}x${alphaSize}" xc: -fx "$outsideExpression ? 0 : i/w" \) \( -size "${alphaSize}x${alphaSize}" xc: -fx "$outsideExpression ? 1 : 0.2" \) \( -size "${alphaSize}x${alphaSize}" xc: -fx "$outsideExpression ? 0 : 1-i/w" \) \( -size "${alphaSize}x${alphaSize}" xc: -fx "$alphaExpression" \) -set colorspace sRGB -channel RGBA -combine -depth 8 "${pngOptions[@]}" PNG32:alpha_disk.png

# 1 px red frame around lime, odd size
step "border_frame_odd.png"
magick -size 1021x765 xc:lime -bordercolor red -border 1 -type TrueColor -depth 8 "${pngOptions[@]}" PNG24:border_frame_odd.png

# --- Robustness: sizes ---------------------------------------------------------------------------------------------------

# Wider than GL_MAX_TEXTURE_SIZE (32768 here), red ticks every 1000 px
extremeWidth=40000
extremeHeight=64
extremeTickSpacing=1000
step "extreme_aspect.png"
drawTicks=""
for (( x = 0; x < extremeWidth; x += extremeTickSpacing )); do
	drawTicks+="line $x,0 $x,$(( extremeHeight - 1 )) "
done
magick -size "${extremeWidth}x${extremeHeight}" -define gradient:direction=East gradient:black-white -stroke red -strokewidth 1 +antialias -draw "$drawTicks" -type TrueColor -depth 8 "${pngOptions[@]}" PNG24:extreme_aspect.png

step "tiny_1x1.png, tiny_3x2.png"
magick xc:red -type TrueColor -depth 8 "${pngOptions[@]}" PNG24:tiny_1x1.png
magick \( xc:red xc:lime xc:blue +append \) \( xc:yellow xc:cyan xc:magenta +append \) -append -type TrueColor -depth 8 "${pngOptions[@]}" PNG24:tiny_3x2.png

# --- Bit depths and formats ----------------------------------------------------------------------------------------------

step "gradient16.png"
magick -size 4096x256 -define gradient:direction=East gradient:black-white -depth 16 "${pngOptions[@]}" PNG48:gradient16.png

formatSize=256
step "palette8.png, gray8.png"
magick \( -size "${formatSize}x${formatSize}" xc: -fx "i/w" \) \( -size "${formatSize}x${formatSize}" xc: -fx "j/h" \) \( -size "${formatSize}x${formatSize}" xc: -fx "0.5" \) -set colorspace sRGB -combine +dither -colors 64 "${pngOptions[@]}" PNG8:palette8.png
magick -size "${formatSize}x${formatSize}" -define gradient:direction=East gradient:black-white -type Grayscale -depth 8 "${pngOptions[@]}" "${pngGray8Options[@]}" gray8.png

# Small RGBA source for the PNM/PAM loader: gradient with a transparent corner
portableWidth=64
portableHeight=48
step "render_rgba.pam, render_rgb.ppm, render_gray.pgm"
magick \( -size "${portableWidth}x${portableHeight}" xc: -fx "i/w" \) \( -size "${portableWidth}x${portableHeight}" xc: -fx "j/h" \) \( -size "${portableWidth}x${portableHeight}" xc: -fx "0.25" \) \( -size "${portableWidth}x${portableHeight}" xc: -fx "(i < w/4 && j < h/4) ? 0 : 1" \) -set colorspace sRGB -channel RGBA -combine -depth 8 -strip portable_source.png
magick portable_source.png -depth 8 PAM:render_rgba.pam
magick portable_source.png -alpha off -depth 8 PPM:render_rgb.ppm
magick portable_source.png -alpha off -colorspace Gray -depth 8 PGM:render_gray.pgm
rm portable_source.png

# BMP: 24 bit bottom-up written by ImageMagick, 32 bit top-down (negative height) written by hand, 8 bit RLE for the loader fallback
bmpWidth=257
bmpHeight=131
bmpRedExpression="i/(w-1)"
bmpGreenExpression="j/(h-1)"
bmpBlueExpression="0.5+0.5*sin(i*0.3)*cos(j*0.2)"
step "bmp24_bottomup.bmp, bmp32_topdown.bmp, bmp_rle8.bmp"
fx_rgb $bmpWidth $bmpHeight "$bmpRedExpression" "$bmpGreenExpression" "$bmpBlueExpression" bmp_source.png
magick bmp_source.png -type TrueColor BMP3:bmp24_bottomup.bmp
bmpFileHeaderSize=14
bmpInfoHeaderSize=40
bmpPixelsOffset=$(( bmpFileHeaderSize + bmpInfoHeaderSize ))
bmp32BytesPerPixel=4
bmp32ImageSize=$(( bmpWidth * bmpHeight * bmp32BytesPerPixel ))
bmpPixelsPerMeter=2835
{
	printf 'BM'
	le32 $(( bmpPixelsOffset + bmp32ImageSize ))
	le32 0
	le32 $bmpPixelsOffset
	le32 $bmpInfoHeaderSize
	le32 $bmpWidth
	le32 $(( -bmpHeight ))
	le16 1
	le16 32
	le32 0
	le32 $bmp32ImageSize
	le32 $bmpPixelsPerMeter
	le32 $bmpPixelsPerMeter
	le32 0
	le32 0
	magick bmp_source.png -alpha opaque -depth 8 BGRA:-
} > bmp32_topdown.bmp
rm bmp_source.png
bmpRleSize=64
magick -size "${bmpRleSize}x${bmpRleSize}" -define gradient:direction=East gradient:red-blue +dither -colors 16 -compress RLE BMP3:bmp_rle8.bmp

# PNG content behind a .jpg extension: the signature must win over the extension
step "png_named.jpg"
magick -size 64x64 -define gradient:direction=South gradient:orange-purple -type TrueColor -depth 8 "${pngOptions[@]}" PNG24:png_named.jpg

# --- EXIF orientation ----------------------------------------------------------------------------------------------------

# Upright motif: black "F" on white with red/lime/blue/yellow corners, asymmetric under every rotation and mirroring
orientationWidth=300
orientationHeight=200
orientationCorner=30
step "orientation_1.jpg ... orientation_8.jpg"
magick -size "${orientationWidth}x${orientationHeight}" xc:white \
	-fill black -draw "rectangle 110,40 135,160 rectangle 110,40 200,62 rectangle 110,90 180,110" \
	-fill red -draw "rectangle 0,0 $(( orientationCorner - 1 )),$(( orientationCorner - 1 ))" \
	-fill lime -draw "rectangle $(( orientationWidth - orientationCorner )),0 $(( orientationWidth - 1 )),$(( orientationCorner - 1 ))" \
	-fill blue -draw "rectangle 0,$(( orientationHeight - orientationCorner )) $(( orientationCorner - 1 )),$(( orientationHeight - 1 ))" \
	-fill yellow -draw "rectangle $(( orientationWidth - orientationCorner )),$(( orientationHeight - orientationCorner )) $(( orientationWidth - 1 )),$(( orientationHeight - 1 ))" \
	-type TrueColor -depth 8 "${pngOptions[@]}" PNG24:orientation_upright.png
# Stored pixels are the inverse of what the viewer must apply for each orientation value
storeTransforms=("" "" "-flop" "-rotate 180" "-flip" "-transpose" "-rotate -90" "-transverse" "-rotate 90")
for orientation in 1 2 3 4 5 6 7 8; do
	# Odd orientations big endian, even ones little endian, so both byte orders are covered
	if (( orientation % 2 == 1 )); then
		byteOrder="MM"
	else
		byteOrder="II"
	fi
	# shellcheck disable=SC2086
	magick orientation_upright.png ${storeTransforms[$orientation]} "${jpegOptions[@]}" JPEG:orientation_stored.jpg
	tiffBlock="$(exif_orientation_block $orientation $byteOrder)"
	insert_exif orientation_stored.jpg "$tiffBlock" "orientation_${orientation}.jpg"
done
rm orientation_stored.jpg

# --- Broken files ----------------------------------------------------------------------------------------------------------

step "truncated.jpg, empty.png"
magick orientation_upright.png "${jpegOptions[@]}" JPEG:truncated_source.jpg
truncatedSize=$(( $(stat -c %s truncated_source.jpg) / 2 ))
head -c $truncatedSize truncated_source.jpg > truncated.jpg
rm truncated_source.jpg
: > empty.png

# IFD0 claims 256 entries in a block that holds one, the orientation entry claims 65536 values at an offset far outside the block
step "exif_broken.jpg, exif_broken_ifd_offset.jpg"
brokenSize=64
magick -size "${brokenSize}x${brokenSize}" -define gradient:direction=South gradient:teal-white "${jpegOptions[@]}" JPEG:broken_source.jpg
insert_exif broken_source.jpg 'MM\x00\x2a\x00\x00\x00\x08\x01\x00\x01\x12\x00\x03\x00\x01\x00\x00\xff\xff\xff\xf0\x7f\xff\xff\xff' exif_broken.jpg
# IFD0 offset points far behind the end of the block
insert_exif broken_source.jpg 'II\x2a\x00\xf0\xff\xff\x7f\x01\x00\x12\x01\x03\x00\x01\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00' exif_broken_ifd_offset.jpg
rm broken_source.jpg

# --- Info line (umlauts in file names) -------------------------------------------------------------------------------------

step "Übersicht_ä.png"
magick -density 72 -font DejaVu-Sans -size 320x200 xc:'#303848' -fill white -pointsize 24 -gravity center -annotate +0+0 "Übersicht ä" -type TrueColor -depth 8 "${pngOptions[@]}" "PNG24:Übersicht_ä.png"

echo "Done."
