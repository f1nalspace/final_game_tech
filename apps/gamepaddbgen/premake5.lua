project "gamepaddbgen"
   kind "ConsoleApp"

   language "C"
   cdialect "C99"

   includedirs { "../../" }

   files { "gamepaddbgen.c" }

   filter "system:linux"
      links { "m" }
