I tested the DirectInput8 implementation on my Win11 VM and it does work not correctly.
Note that the gampad layout is inspired by XInput / XBox-Controller, so action-buttons are (Y up, X left, A down, B right).
Also i added unions to fplGameControllerState so we can map those 4 buttons by its directional name as well.

# 1. DIJoyStat not correct

- The fpl__DIJoyState is NOT the same as DIJOYSTATE struct:
    - Missing LONG rglSlider[2] after lRz
    - Only one POV -> DWORD rgdwPOV[4] after rglSlider   
   
- The fpl__DIJoy_Objects mapping is not complete and looks wrong, compared against other DInput implementations (ignore ofs)

A year ago i got a MR, that i could not use directly - but the user was stating DInput was working.
Double check and compare againt the our implementation, especially the DIOBJECTDATAFORMAT table:

DIOBJECTDATAFORMAT objdatafmt[] = {
{ &GUID_XAxis, 0x00000000, 0x80ffff03, 0x00000100 },
{ &GUID_YAxis, 0x00000004, 0x80ffff03, 0x00000100 },
{ &GUID_ZAxis, 0x00000008, 0x80ffff03, 0x00000100 },
{ &GUID_RxAxis, 0x0000000c, 0x80ffff03, 0x00000100 },
{ &GUID_RyAxis, 0x00000010, 0x80ffff03, 0x00000100 },
{ &GUID_RzAxis, 0x00000014, 0x80ffff03, 0x00000100 },
{ &GUID_Slider, 0x00000018, 0x80ffff03, 0x00000100 },
{ &GUID_Slider, 0x0000001c, 0x80ffff03, 0x00000100 },
{ &GUID_POV, 0x00000020, 0x80ffff10, 0x00000000 },
{ &GUID_POV, 0x00000024, 0x80ffff10, 0x00000000 },
{ &GUID_POV, 0x00000028, 0x80ffff10, 0x00000000 },
{ &GUID_POV, 0x0000002c, 0x80ffff10, 0x00000000 },
// The 32-buttons, each is
{ 0, 0x00000030, 0x80ffff0c, 0x00000000 },
// Then there are 3 additional full mappings, but without buttons
};

See `https://github.com/f1nalspace/final_game_tech/pull/174/changes/34e9f54ac1eb5f9d53ce7ae8b4a7bd68dbf23ded#diff-7bb6c6723b9f037cdbf55dee20d2d5f9502aac4133df58d31aeb9fd696e52f03` for the diff.

# 2. Drop DIPROP_RANGE, if possible

- Think about not using the DIPROP_RANGE anymore and use the original DInput range [0, +32767, +65535] instead

# 3. Invalid mappings

- Only the following button mappings works
    - DPad (Up, Down, Left, Right)
    - Left-Shoulder
    - Right-Shoulder
    - Y-Button
       
- Right stick is mapped to right-trigger -> wrong
- Select is mapped to Left-Stick button -> wrong
- Start is mapped to Right-Stick button -> wrong
- Left-Stick button is not mapped at all -> missing
- Right-Stick button is not mapped at all -> missing
- Left-rigger is mapped to Select button -> wrong (digital vs analog)
- Right-rigger is mapped to Start button -> wrong (digital vs analog)

- When i start the FPL_Input demo and enable the gamepad, both the left-trigger and right-trigger are shown half, so it is 0.5 -> wrong

# 4. Slot device name not UTF-8

- The slot->deviceName should be UTF-8, not ASCII
- Use fplWideStringToUTF8String and lstrlenW to convert the ddi->tszProductName into UTF-8

