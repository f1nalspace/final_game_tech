project "gamepaddbgen"
   kind "ConsoleApp"

   language "C"
   cdialect "C99"

   includedirs { "../../", "../../examples/" }

   files { "gamepaddbgen.c" }

   filter "system:linux"
      links { "m", "pthread", "dl" }
