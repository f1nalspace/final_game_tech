#!/usr/bin/env python3
# Assembles an Apple icon file (.icns) for the FPL_ImageViewer ICNS loader tests, with the Python standard library only.
#
# Usage: make_icns.py <output.icns> <type>=<file> ...
#   ic07 ... ic14, icp4 ... icp6: the file is stored as it is (PNG or JPEG 2000)
#   ic04, ic05: the file holds raw RGBA of the icon size, stored as packed ARGB
#   is32, il32, ih32, it32: the file holds raw RGBA of the icon size, stored as packed RGB plus its 8 bit mask entry

import struct
import sys

ICON_SIDES = {"ic04": 16, "ic05": 32, "is32": 16, "il32": 32, "ih32": 48, "it32": 128}
MASK_TYPES = {"is32": b"s8mk", "il32": b"l8mk", "ih32": b"h8mk", "it32": b"t8mk"}
ENTRY_HEADER_SIZE = 8
RGBA_CHANNELS = 4
# Packed channels: a control byte below 0x80 copies the next control + 1 bytes, from 0x80 on the next byte repeats control - 0x80 + 3 times
RUN_FLAG = 0x80
SHORTEST_RUN = 3
LONGEST_RUN = 130
LONGEST_LITERAL = 128
IT32_PREFIX = b"\0\0\0\0"
ARGB_PREFIX = b"ARGB"


def pack_channel(values):
    packed = bytearray()
    index = 0
    count = len(values)
    while index < count:
        runLength = 1
        while index + runLength < count and runLength < LONGEST_RUN and values[index + runLength] == values[index]:
            runLength += 1
        if runLength >= SHORTEST_RUN:
            packed.append(RUN_FLAG + runLength - SHORTEST_RUN)
            packed.append(values[index])
            index += runLength
            continue
        literal = bytearray()
        while index < count and len(literal) < LONGEST_LITERAL:
            # A literal ends where a run of three starts
            isRunAhead = index + 2 < count and values[index] == values[index + 1] == values[index + 2]
            if isRunAhead:
                break
            literal.append(values[index])
            index += 1
        packed.append(len(literal) - 1)
        packed += literal
    return bytes(packed)


def split_rgba(data, side, path):
    expectedSize = side * side * RGBA_CHANNELS
    if len(data) != expectedSize:
        sys.exit(f"{path}: {len(data)} bytes, a {side} x {side} RGBA icon has {expectedSize}")
    red = data[0::RGBA_CHANNELS]
    green = data[1::RGBA_CHANNELS]
    blue = data[2::RGBA_CHANNELS]
    alpha = data[3::RGBA_CHANNELS]
    return red, green, blue, alpha


def main():
    if len(sys.argv) < 3:
        sys.exit("Usage: make_icns.py <output.icns> <type>=<file> ...")
    entries = []
    for argument in sys.argv[2:]:
        iconType, path = argument.split("=", 1)
        with open(path, "rb") as file:
            data = file.read()
        typeBytes = iconType.encode("ascii")
        if iconType in ("ic04", "ic05"):
            red, green, blue, alpha = split_rgba(data, ICON_SIDES[iconType], path)
            payload = ARGB_PREFIX + pack_channel(alpha) + pack_channel(red) + pack_channel(green) + pack_channel(blue)
            entries.append((typeBytes, payload))
        elif iconType in MASK_TYPES:
            red, green, blue, alpha = split_rgba(data, ICON_SIDES[iconType], path)
            prefix = IT32_PREFIX if iconType == "it32" else b""
            payload = prefix + pack_channel(red) + pack_channel(green) + pack_channel(blue)
            entries.append((typeBytes, payload))
            entries.append((MASK_TYPES[iconType], bytes(alpha)))
        else:
            entries.append((typeBytes, data))
    body = b"".join(entryType + struct.pack(">I", len(payload) + ENTRY_HEADER_SIZE) + payload for entryType, payload in entries)
    with open(sys.argv[1], "wb") as file:
        file.write(b"icns" + struct.pack(">I", len(body) + ENTRY_HEADER_SIZE) + body)


main()
