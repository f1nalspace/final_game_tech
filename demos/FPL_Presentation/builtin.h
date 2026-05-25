#pragma once

#include "fonts.h"
#include "presentation.h"

namespace BuiltinFonts {
    static const Resource Debug = Resource::CreateFromMemory("Debug", ptr_bitstreamVerySansFontData, sizeOf_bitstreamVerySansFontData);
    static const Resource Arimo = Resource::CreateFromMemory("Arimo", ptr_arimoRegularFontData, sizeOf_arimoRegularFontData);
    static const Resource SulphurPoint = Resource::CreateFromMemory("Sulphur Point", ptr_sulphurPointRegularData, sizeOf_sulphurPointRegularData);
    static const Resource BitStreamVerySans = Resource::CreateFromMemory("Bitstream Vera Sans", ptr_bitstreamVerySansFontData, sizeOf_bitstreamVerySansFontData);
};

namespace BuiltinImages {
	static Resource FPLLogo128x128 = Resource::CreateFromMemory("FPL Logo 128x128", ptr_fplLogo128x128ImageData, sizeOf_fplLogo128x128ImageData);
	static Resource FPLLogo512x512 = Resource::CreateFromMemory("FPL Logo 512x512", ptr_fplLogo512x512ImageData, sizeOf_fplLogo512x512ImageData);

	static const Resource All[] = {
		FPLLogo128x128,
		FPLLogo512x512,
	};
}