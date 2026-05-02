I tested the DirectInput8 implementation on my Win11 VM and it does work not correctly.
Note that the gampad layout is inspired by XInput / XBox-Controller, so action-buttons are (Y up, X left, A down, B right).
Also i added unions to fplGameControllerState so we can map those 4 buttons by its directional name as well.

Use `https://raw.githubusercontent.com/ColleagueRiley/minigamepad/refs/heads/main/minigamepad.h` as a full reference.

# 0. No dinput.h inclusion

You can safely include `dinput.h` library, so can remove our custom types and defines and use all dinput stuff - but you have to fix up the runtime linked function definitions!

# 1. DIJoyStat not correct

- The fpl__DIJoyState is NOT the same as DIJOYSTATE struct:
    - Missing LONG rglSlider[2] after lRz
    - Only one POV -> DWORD rgdwPOV[4] after rglSlider
   
- The fpl__DIJoy_Objects mapping is not complete and looks wrong, compare against other DInput implementations

See `https://github.com/f1nalspace/final_game_tech/pull/174/changes/34e9f54ac1eb5f9d53ce7ae8b4a7bd68dbf23ded#diff-7bb6c6723b9f037cdbf55dee20d2d5f9502aac4133df58d31aeb9fd696e52f03` for the diff.

# 2. Drop DIPROP_RANGE, if possible

- Do not use the DIPROP_RANGE anymore and use the original DInput range [0, +32767, +65535] instead

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
- Both the left-trigger and right-trigger are 0.5, even though it is not moved at all -> wrong

# 4. Slot device name not UTF-8

- The slot->deviceName should be UTF-8, not ASCII
- Use fplWideStringToUTF8String and lstrlenW to convert the ddi->tszProductName into UTF-8

