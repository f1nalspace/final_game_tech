project "mathtest"
   kind "ConsoleApp"

   language "C"
   cdialect "C99"

   includedirs { "../../", "../../demos/additions" }

   files { "mathtest.c" }

   filter "system:linux"
      links { "m" }
