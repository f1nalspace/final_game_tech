I want you to extent the final tile trace demo, so that we can visually control the demo and switch on various scenarios.

- Add a very basic font rendering that uses lines for drawing the ascii-characters in legacy OpenGL with just set of alpha and numeric characters + underscore and point.
	- Define each char as vertex points in a C17 style static array
	- Render the char as simple as possible, no bezier curves, no slopes, just lines.
	- Scale and line thickness can be passed to the actual text render function
	- Legacy opengl

- Extent the demo to a very minimalistic immediate UI that can render labels, buttons, checkboxes, radiobuttons.

- Goal is to control the FTT update of the demo, so you can pause the tile-trace process, continue or even single-step it.

- In addition i want to have multiple scenarios/tilemaps you can load from radio-button switching, for now:
	- 0 = A simple version version of a tilemap that is much smaller than the current one
	- 1 = The current one
	- 2 = A version that random generates a tilemap (e.g. dungeon style) of the size of the current one
	
---

I want to extent the "final_tiletrace.h" library to support slope tiles.
Right now a tilemap in that library with value 1 is treated as a full solid tile and zero is treated as empty space.
We leave that uint8_t as the type, but we the use following enum to map each value into its respective tile type.
For that i added an enum fttTileType that contains all 6 types i want to support (that may be extended later to more slope tiles).

Extent the demo to allow for more scenarios:
- 3 = A simple tile map that uses a few slopes
- 4 = A extended version of the large tilemap version 1, but uses slopes on every corner/edge
- 5 = A version that is randomly generated and uses slopes, of the size of scenario 1
