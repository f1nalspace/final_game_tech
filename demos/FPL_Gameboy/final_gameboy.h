#ifndef FINAL_GAMEBOY_HEADER
#define FINAL_GAMEBOY_HEADER

#include <stdint.h>

enum fgbMemoryControllerType {
	fgbMemoryControllerType_Unknown = 0,
	fgbMemoryControllerType_ROM,
	fgbMemoryControllerType_MBC1,
	fgbMemoryControllerType_MBC2,
	fgbMemoryControllerType_MMM01,
	fgbMemoryControllerType_MBC3,
	fgbMemoryControllerType_MBC5,
	fgbMemoryControllerType_MBC6,
	fgbMemoryControllerType_MBC7,
	fgbMemoryControllerType_HUC3,
	fgbMemoryControllerType_HUC1,
	fgbMemoryControllerType_Count,
};

enum fgbRomSizeType {
	fgbRomSizeType_2_Banks_32KB = 0x00,
	fgbRomSizeType_4_Banks_64KB = 0x01,
	fgbRomSizeType_8_Banks_128KB = 0x02,
	fgbRomSizeType_16_Banks_256KB = 0x03,
	fgbRomSizeType_32_Banks_512KB = 0x04,
	fgbRomSizeType_64_Banks_1024KB = 0x05,
	fgbRomSizeType_128_Banks_2048KB = 0x06,
	fgbRomSizeType_256_Banks_4098KB = 0x07,
	fgbRomSizeType_512_Banks_8192KB = 0x08,
	fgbRomSizeType_72_Banks_1152KB = 0x52,
	fgbRomSizeType_80_Banks_1280KB = 0x53,
	fgbRomSizeType_96_Banks_1536KB = 0x54,
};

enum fgbRamSizeType {
	fgbRamSizeType_NoRam = 0x00,
	fgbRamSizeType_Unused = 0x01,
	fgbRamSizeType_1_Banks_8KB = 0x02,
	fgbRamSizeType_4_Banks_32KB = 0x03,
	fgbRamSizeType_16_Banks_128KB = 0x04,
	fgbRamSizeType_8_Banks_64KB = 0x05,
};

enum fgbCartridgeType {
	fgbCartridgeType_ROM = 0x00,
	fgbCartridgeType_MBC1 = 0x01,
	fgbCartridgeType_MBC1_RAM = 0x02,
	fgbCartridgeType_MBC1_RAM_BATTERY = 0x03,
	fgbCartridgeType_MBC2 = 0x05,
	fgbCartridgeType_MBC2_BATTERY = 0x06,
	fgbCartridgeType_ROM_BATTERY = 0x08,
	fgbCartridgeType_ROM_RAM_BATTERY = 0x09,
	fgbCartridgeType_MMM01 = 0x0B,
	fgbCartridgeType_MMM01_RAM = 0x0C,
	fgbCartridgeType_MMM01_RAM_BATTERY = 0x0D,
	fgbCartridgeType_MBC3_TIMER_BATTERY = 0x0F,
	fgbCartridgeType_MBC3_TIMER_RAM_BATTERY = 0x10,
	fgbCartridgeType_MBC3 = 0x11,
	fgbCartridgeType_MBC3_RAM = 0x12,
	fgbCartridgeType_MBC3_RAM_BATTERY = 0x13,
	fgbCartridgeType_MBC5 = 0x19,
	fgbCartridgeType_MBC5_RAM = 0x1A,
	fgbCartridgeType_MBC5_RAM_BATTERY = 0x1B,
	fgbCartridgeType_MBC5_RUMBLE = 0x1C,
	fgbCartridgeType_MBC5_RUMBLE_RAM = 0x1D,
	fgbCartridgeType_MBC5_RUMBLE_RAM_BATTERY = 0x1E,
	fgbCartridgeType_MBC6 = 0x20,
	fgbCartridgeType_MBC7_SENSOR_RUMBLE_RAM_BATTERY = 0x22,
	fgbCartridgeType_POCKET_CAMERA = 0xFC,
	fgbCartridgeType_BANDAI_TAMA5 = 0xFD,
	fgbCartridgeType_HUC3 = 0xFE,
	fgbCartridgeType_HUC1_RAM_BATTERY = 0xFF,
};

enum fgbCoreType {
	fgbCoreType_GB = 0,
	fgbCoreType_GBC = 1,
	fgbCoreType_GBC_GB = 2,
	fgbCoreType_SGB = 3,
	fgbCoreType_Count,
};

#define FGB_KILOBYTES(kb) ((kb) * 1024)

#define FGB_BANK_SIZE FGB_KILOBYTES(16)
#define FGB_MIN_CARTRIGE_SIZE (2 * FGB_BANK_SIZE)
#define FGB_MAX_CARTRIGE_SIZE (128 * FGB_BANK_SIZE)

#define FGB_MIN_EXTERNAL_RAM FGB_KILOBYTES(8)
#define FGB_MAX_EXTERNAL_RAM FGB_KILOBYTES(32)

struct fgbCartridge {
	uint8_t rom[FGB_MAX_CARTRIGE_SIZE];
	char title[24];
	uint32_t size;
	uint32_t romBankCount;
	uint32_t sramBankCount;
	fgbCartridgeType cartridgetype;
	fgbCoreType coreType;
	fgbRamSizeType ramSizeType;
	fgbMemoryControllerType memoryControllerType;
	bool isValid;
};

enum fgbCartridgeFeatures {
	fgbCartridgeFeatures_None = 0,
	fgbCartridgeFeatures_RAM = 1 << 0,
	fgbCartridgeFeatures_BATTERY = 1 << 1,
	fgbCartridgeFeatures_TIMER = 1 << 2,
	fgbCartridgeFeatures_RUMBLE = 1 << 3,
	fgbCartridgeFeatures_SENSOR = 1 << 4,
};

struct fgbMemory {
	uint8_t rom[FGB_MAX_CARTRIGE_SIZE];
	uint8_t sram[FGB_MAX_EXTERNAL_RAM];
};

#define FGB_DISPLAY_WIDTH 160
#define FGB_DISPLAY_HEIGHT 144
#define FGB_DISPLAY_PIXELS_LENGTH (FGB_DISPLAY_WIDTH * FGB_DISPLAY_HEIGHT)

struct fgbPPU {
	uint32_t pixels[FGB_DISPLAY_PIXELS_LENGTH];
	uint8_t vram[0x2000];
};

#define fgbFlagBit_Carry 4
#define fgbFlagBit_HalfCarry 5
#define fgbFlagBit_Subtract 6
#define fgbFlagBit_Zero 7

union fgbFlagsRegister {
	struct {
		uint8_t zero : 1;
		uint8_t subtract : 1;
		uint8_t halfCarry : 1;
		uint8_t carry : 1;
		uint8_t unused : 4;
	};
	uint8_t value;
};

struct fgbRegister {
	union {
		struct {
			fgbFlagsRegister f;
			uint8_t a;
		};
		uint16_t af;
	};

	union {
		struct {
			uint8_t c;
			uint8_t b;
		};
		uint16_t bc;
	};

	union {
		struct {
			uint8_t e;
			uint8_t d;
		};
		uint16_t de;
	};

	union {
		struct {
			uint8_t l;
			uint8_t h;
		};
		uint16_t hl;
	};
	uint16_t sp;
	uint16_t pc;
};

struct fgbCPU {
	fgbRegister reg;
};

#endif // FINAL_GAMEBOY_HEADER

#if defined(FINAL_GAMEBOY_IMPLEMENTATION) && !defined(FINAL_GAMEBOY_IMPLEMENTED)
#define FINAL_GAMEBOY_IMPLEMENTED

static const uint8_t fgb__LicenseLogo[48] = {
	0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
	0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
	0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
	0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
	0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
	0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
};

struct fgb__RomSizeTable {
	const char *names[256];
	uint16_t counts[256];

	fgb__RomSizeTable() {
		memset(names, 0, sizeof(names));
		memset(counts, 0, sizeof(counts));

		names[fgbRomSizeType_2_Banks_32KB] = "2 Banks; 32 KB";
		names[fgbRomSizeType_4_Banks_64KB] = "4 Banks; 64 KB";
		names[fgbRomSizeType_8_Banks_128KB] = "8 Banks; 128 KB";
		names[fgbRomSizeType_16_Banks_256KB] = "16 Banks; 256 KB";
		names[fgbRomSizeType_32_Banks_512KB] = "32 Banks; 512 KB";
		names[fgbRomSizeType_64_Banks_1024KB] = "64 Banks; 1 MB";
		names[fgbRomSizeType_128_Banks_2048KB] = "128 Banks; 2 MB";
		names[fgbRomSizeType_256_Banks_4098KB] = "256 Banks; 4 MB";
		names[fgbRomSizeType_512_Banks_8192KB] = "512 Banks; 8 MB";
		names[fgbRomSizeType_72_Banks_1152KB] = "72 Banks; 1.1 MB";
		names[fgbRomSizeType_80_Banks_1280KB] = "80 Banks; 1.2 MB";
		names[fgbRomSizeType_96_Banks_1536KB] = "96 Banks; 1.5 MB";

		counts[fgbRomSizeType_2_Banks_32KB] = 2;
		counts[fgbRomSizeType_4_Banks_64KB] = 4;
		counts[fgbRomSizeType_8_Banks_128KB] = 8;
		counts[fgbRomSizeType_16_Banks_256KB] = 16;
		counts[fgbRomSizeType_32_Banks_512KB] = 32;
		counts[fgbRomSizeType_64_Banks_1024KB] = 64;
		counts[fgbRomSizeType_128_Banks_2048KB] = 128;
		counts[fgbRomSizeType_256_Banks_4098KB] = 256;
		counts[fgbRomSizeType_512_Banks_8192KB] = 512;
		counts[fgbRomSizeType_72_Banks_1152KB] = 72;
		counts[fgbRomSizeType_80_Banks_1280KB] = 80;
		counts[fgbRomSizeType_96_Banks_1536KB] = 96;
	}
};

struct fgb__RamSizeTable {
	const char *names[256];
	uint16_t counts[256];

	fgb__RamSizeTable() {
		memset(names, 0, sizeof(names));
		memset(counts, 0, sizeof(counts));

		names[fgbRamSizeType_NoRam] = "No RAM";
		names[fgbRamSizeType_Unused] = "Unused";
		names[fgbRamSizeType_1_Banks_8KB] = "1 Banks; 8 KB";
		names[fgbRamSizeType_4_Banks_32KB] = "4 Banks; 32 KB";
		names[fgbRamSizeType_16_Banks_128KB] = "16 Banks; 128 KB";
		names[fgbRamSizeType_8_Banks_64KB] = "8 Banks; 64 KB";

		counts[fgbRamSizeType_NoRam] = 0;
		counts[fgbRamSizeType_Unused] = 0;
		counts[fgbRamSizeType_1_Banks_8KB] = 1;
		counts[fgbRamSizeType_4_Banks_32KB] = 4;
		counts[fgbRamSizeType_16_Banks_128KB] = 16;
		counts[fgbRamSizeType_8_Banks_64KB] = 8;
	}
};

struct fgb__LicenseCodeTable {
	const char *oldNames[256];
	const char *newNames[256];

	fgb__LicenseCodeTable() {
		memset(oldNames, 0, sizeof(oldNames));
		memset(newNames, 0, sizeof(newNames));

		// TODO(final): Fill out old license code table (https://raw.githubusercontent.com/gb-archive/salvage/master/txt-files/gbrom.txt)
		oldNames[0x00] = "None";
		oldNames[0x01] = "Nintendo";

		// TODO(final): Fill out new license code table (https://gbdev.io/pandocs/The_Cartridge_Header.html#0144-0145---new-licensee-code)
		newNames[0] = "None";
		newNames[1] = "Nintendo";
	}
};

struct fgb__CoreTypesTable {
	const char *names[fgbCoreType_Count];

	fgb__CoreTypesTable() {
		names[fgbCoreType_GB] = "Gameboy";
		names[fgbCoreType_GBC] = "Gameboy Color";
		names[fgbCoreType_GBC_GB] = "Gameboy Color Mode";
		names[fgbCoreType_SGB] = "Super Gameboy";
	}
};

struct fgb__CartridgeMappingTable {
	fgbMemoryControllerType types[256];
	fgbCartridgeFeatures features[256];
	const char *names[256];

	fgb__CartridgeMappingTable() {
		memset(types, 0, sizeof(types));
		memset(features, 0, sizeof(features));
		memset(names, 0, sizeof(names));

		types[fgbCartridgeType_ROM] = fgbMemoryControllerType_ROM;
		types[fgbCartridgeType_MBC1] = fgbMemoryControllerType_MBC1;
		types[fgbCartridgeType_MBC1_RAM] = fgbMemoryControllerType_MBC1;
		types[fgbCartridgeType_MBC1_RAM_BATTERY] = fgbMemoryControllerType_MBC1;
		types[fgbCartridgeType_MBC2] = fgbMemoryControllerType_MBC2;
		types[fgbCartridgeType_MBC2_BATTERY] = fgbMemoryControllerType_MBC2;
		types[fgbCartridgeType_ROM_BATTERY] = fgbMemoryControllerType_ROM;
		types[fgbCartridgeType_ROM_RAM_BATTERY] = fgbMemoryControllerType_ROM;
		types[fgbCartridgeType_MMM01] = fgbMemoryControllerType_MMM01;
		types[fgbCartridgeType_MMM01_RAM] = fgbMemoryControllerType_MMM01;
		types[fgbCartridgeType_MMM01_RAM_BATTERY] = fgbMemoryControllerType_MMM01;
		types[fgbCartridgeType_MBC3_TIMER_BATTERY] = fgbMemoryControllerType_MBC3;
		types[fgbCartridgeType_MBC3_TIMER_RAM_BATTERY] = fgbMemoryControllerType_MBC3;
		types[fgbCartridgeType_MBC3] = fgbMemoryControllerType_MBC3;
		types[fgbCartridgeType_MBC3_RAM] = fgbMemoryControllerType_MBC3;
		types[fgbCartridgeType_MBC3_RAM_BATTERY] = fgbMemoryControllerType_MBC3;
		types[fgbCartridgeType_MBC5] = fgbMemoryControllerType_MBC5;
		types[fgbCartridgeType_MBC5_RAM] = fgbMemoryControllerType_MBC5;
		types[fgbCartridgeType_MBC5_RAM_BATTERY] = fgbMemoryControllerType_MBC5;
		types[fgbCartridgeType_MBC5_RUMBLE] = fgbMemoryControllerType_MBC5;
		types[fgbCartridgeType_MBC5_RUMBLE_RAM] = fgbMemoryControllerType_MBC5;
		types[fgbCartridgeType_MBC5_RUMBLE_RAM_BATTERY] = fgbMemoryControllerType_MBC5;
		types[fgbCartridgeType_MBC6] = fgbMemoryControllerType_MBC6;
		types[fgbCartridgeType_MBC7_SENSOR_RUMBLE_RAM_BATTERY] = fgbMemoryControllerType_MBC7;
		types[fgbCartridgeType_POCKET_CAMERA] = fgbMemoryControllerType_ROM; // Unknown
		types[fgbCartridgeType_BANDAI_TAMA5] = fgbMemoryControllerType_ROM; // Unknown
		types[fgbCartridgeType_HUC3] = fgbMemoryControllerType_HUC3;
		types[fgbCartridgeType_HUC1_RAM_BATTERY] = fgbMemoryControllerType_HUC1;
		
		names[fgbCartridgeType_ROM] = "ROM";
		names[fgbCartridgeType_MBC1] = "MBC1";
		names[fgbCartridgeType_MBC1_RAM] = "MBC1+RAM";
		names[fgbCartridgeType_MBC1_RAM_BATTERY] = "MBC1+RAM+BATTERY";
		names[fgbCartridgeType_MBC2] = "MBC2";
		names[fgbCartridgeType_MBC2_BATTERY] = "MBC2+BATTERY";
		names[fgbCartridgeType_ROM_BATTERY] = "ROM+BATTERY";
		names[fgbCartridgeType_ROM_RAM_BATTERY] = "ROM+RAM+BATTERY";
		names[fgbCartridgeType_MMM01] = "MMM01";
		names[fgbCartridgeType_MMM01_RAM] = "MMM01+RAM";
		names[fgbCartridgeType_MMM01_RAM_BATTERY] = "MMM01+RAM+BATTERY";
		names[fgbCartridgeType_MBC3_TIMER_BATTERY] = "MBC3+TIMER+BATTERY";
		names[fgbCartridgeType_MBC3_TIMER_RAM_BATTERY] = "MBC3+TIMER+RAM+BATTERY";
		names[fgbCartridgeType_MBC3] = "MBC3";
		names[fgbCartridgeType_MBC3_RAM] = "MBC3+RAM";
		names[fgbCartridgeType_MBC3_RAM_BATTERY] = "MBC3+RAM+BATTERY";
		names[fgbCartridgeType_MBC5] = "MBC5";
		names[fgbCartridgeType_MBC5_RAM] = "MBC5+RAM";
		names[fgbCartridgeType_MBC5_RAM_BATTERY] = "MBC5+RAM+BATTERY";
		names[fgbCartridgeType_MBC5_RUMBLE] = "MBC5+RUMBLE";
		names[fgbCartridgeType_MBC5_RUMBLE_RAM] = "MBC5+RUMBLE+RAM";
		names[fgbCartridgeType_MBC5_RUMBLE_RAM_BATTERY] = "MBC5+RUMBLE+RAM+BATTERY";
		names[fgbCartridgeType_MBC6] = "MBC6";
		names[fgbCartridgeType_MBC7_SENSOR_RUMBLE_RAM_BATTERY] = "MBC7+SENSOR+RUMBLE+RAM+BATTERY";
		names[fgbCartridgeType_POCKET_CAMERA] = "POCKET+CAMERA";
		names[fgbCartridgeType_BANDAI_TAMA5] = "Bandai TAMA5";
		names[fgbCartridgeType_HUC3] = "HuC3";
		names[fgbCartridgeType_HUC1_RAM_BATTERY] = "HuC1+RAM+BATTERY";

		features[fgbCartridgeType_ROM] = fgbCartridgeFeatures_None;
		features[fgbCartridgeType_MBC1] = fgbCartridgeFeatures_None;
		features[fgbCartridgeType_MBC1_RAM] = fgbCartridgeFeatures_RAM;
		features[fgbCartridgeType_MBC1_RAM_BATTERY] = (fgbCartridgeFeatures)(fgbCartridgeFeatures_RAM | fgbCartridgeFeatures_BATTERY);
		features[fgbCartridgeType_MBC2] = fgbCartridgeFeatures_None;
		features[fgbCartridgeType_MBC2_BATTERY] = fgbCartridgeFeatures_BATTERY;
		features[fgbCartridgeType_ROM_BATTERY] = fgbCartridgeFeatures_BATTERY;
		features[fgbCartridgeType_ROM_RAM_BATTERY] = (fgbCartridgeFeatures)(fgbCartridgeFeatures_RAM | fgbCartridgeFeatures_BATTERY);
		features[fgbCartridgeType_MMM01] = fgbCartridgeFeatures_None;
		features[fgbCartridgeType_MMM01_RAM] = fgbCartridgeFeatures_RAM;
		features[fgbCartridgeType_MMM01_RAM_BATTERY] = (fgbCartridgeFeatures)(fgbCartridgeFeatures_RAM | fgbCartridgeFeatures_BATTERY);
		features[fgbCartridgeType_MBC3_TIMER_BATTERY] = (fgbCartridgeFeatures)(fgbCartridgeFeatures_TIMER | fgbCartridgeFeatures_BATTERY);
		features[fgbCartridgeType_MBC3_TIMER_RAM_BATTERY] = (fgbCartridgeFeatures)(fgbCartridgeFeatures_TIMER | fgbCartridgeFeatures_BATTERY);
		features[fgbCartridgeType_MBC3] = fgbCartridgeFeatures_None;
		features[fgbCartridgeType_MBC3_RAM] = fgbCartridgeFeatures_RAM;
		features[fgbCartridgeType_MBC3_RAM_BATTERY] = (fgbCartridgeFeatures)(fgbCartridgeFeatures_RAM | fgbCartridgeFeatures_BATTERY);
		features[fgbCartridgeType_MBC5] = fgbCartridgeFeatures_None;
		features[fgbCartridgeType_MBC5_RAM] = fgbCartridgeFeatures_RAM;
		features[fgbCartridgeType_MBC5_RAM_BATTERY] = (fgbCartridgeFeatures)(fgbCartridgeFeatures_RAM | fgbCartridgeFeatures_BATTERY);
		features[fgbCartridgeType_MBC5_RUMBLE] = fgbCartridgeFeatures_RUMBLE;
		features[fgbCartridgeType_MBC5_RUMBLE_RAM] = (fgbCartridgeFeatures)(fgbCartridgeFeatures_RUMBLE | fgbCartridgeFeatures_RAM);
		features[fgbCartridgeType_MBC5_RUMBLE_RAM_BATTERY] = (fgbCartridgeFeatures)(fgbCartridgeFeatures_RUMBLE | fgbCartridgeFeatures_RAM | fgbCartridgeFeatures_BATTERY);
		features[fgbCartridgeType_MBC6] = fgbCartridgeFeatures_None;
		features[fgbCartridgeType_MBC7_SENSOR_RUMBLE_RAM_BATTERY] = (fgbCartridgeFeatures)(fgbCartridgeFeatures_SENSOR | fgbCartridgeFeatures_RUMBLE | fgbCartridgeFeatures_RAM | fgbCartridgeFeatures_BATTERY);
		features[fgbCartridgeType_POCKET_CAMERA] = fgbCartridgeFeatures_None;
		features[fgbCartridgeType_BANDAI_TAMA5] = fgbCartridgeFeatures_None;
		features[fgbCartridgeType_HUC3] = fgbCartridgeFeatures_None;
		features[fgbCartridgeType_HUC1_RAM_BATTERY] = (fgbCartridgeFeatures)(fgbCartridgeFeatures_RAM | fgbCartridgeFeatures_BATTERY);
	}
};

static fgb__CartridgeMappingTable fgb__g_cartridgeMappingTable = fgb__CartridgeMappingTable();
static fgb__RomSizeTable fgb__g_romSizeTable = fgb__RomSizeTable();
static fgb__RamSizeTable fgb__g_ramSizeTable = fgb__RamSizeTable();
static fgb__LicenseCodeTable fgb__g_licenseCodeTable = fgb__LicenseCodeTable();
static fgb__CoreTypesTable fgb__g_coreTypesTable = fgb__CoreTypesTable();

#endif // FINAL_GAMEBOY_IMPLEMENTATION