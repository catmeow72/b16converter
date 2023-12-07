#include <Magick++.h>
#include <iostream>
#include <fstream>
#include <map>
#include "palette.h"
#include <string.h>
#include <vector>
#include <string>
#include <exception>
#include <lib.h>
#include <stdlib.h>
#include <filesystem>
#include "win32compat.h"
#include "bitmapx16.h"
using std::vector;
using std::map;
using std::stoi;
using namespace Magick;
void usage() {
	printf("Usage: bmxconvert: [options]\n");
	printf("Options may be:\n");
	printf("-in <input>\n");
	printf("\tSets the input file.\n");
	printf("-out <output>\n");
	printf("\tSets the output file");
	printf("-bpp <bpp>\n");
	printf("\tSets the desired bits per pixel. May be 0 (default), 1, 2, 4, or 8. 0: automatic.\n");
	printf("-significant <color count>\n");
	printf("\tSets the desired number of significant palette entries. Must be at least 0 and at most 256. 0 means automatic. Default: 0\n");
	printf("-resize <width> <height>\n");
	printf("\tResizes the image before converting.\n");
	printf("-dither\n");
	printf("\tEnables dithering of the output\n");
	printf("-border <r> <g> <b>\n");
	printf("\tIf possible, adds a border color with the specified RGB values which are in the range of 0-15.\n");
	printf("-border-idx <palette index>\n");
	printf("\tSets the border color by palette index\n");
	printf("-reverse\n");
	printf("\tConverts to PC formats. Incompatible with -dither, -type, and -significant - they will be ignored.\n");
	printf("-debug <flags>\n");
	printf("\tEnables debugging flags. May contain any number of single-letter debugging flags.\n");
	printf("\tFlags: p = show palette entry, c = show palette closeness lookups\n");
	printf("\tUse an ! before a flag to disable it.\n");
	printf("-compress\n");
	printf("\tCompresses the image with LZSA compression\n");
	printf("-probe\n");
	printf("\tDisplays information about a file, assuming BMX format.");
	printf("-palette-file <file>\n");
	printf("\tSets the palette file to use.\n");
	printf("-help\n");
	printf("\tDisplays this help message.\n");
	exit(1);
}
int main(int argc, char **argv) {
	const char *input = NULL, *output = NULL;
	size_t tw = 0, th = 0; // Target width & height;
	uint8_t tbpp = 0;
	uint16_t tcolorcount = 0;
	const char *error_msg_part = "load the image";
	const char *palette_file = NULL;
	bool dither = false;
	bool reverse = false;
	bool compress = false;
	uint8_t br, bg, bb;
	bool border_set = false;
	bool probe_only = false;
	std::string executable_path = std::filesystem::weakly_canonical(*argv).parent_path().generic_string();
#ifdef _WIN32
	setenv("MAGICK_CODER_MODULE_PATH", executable_path.c_str(), 0);
#endif
	InitializeMagick(*argv);
	argc--;
	argv++;
	while (argc > 0) {
		if (!strcmp(argv[0], "-in")) {
			argc--;
			argv++;
			if (!argc || argv[0][0] == '-') {
				usage();
			}
			input = argv[0];
			argc--;
			argv++;
		} else if (!strcmp(argv[0], "-out")) {

			argc--;
			argv++;
			if (!argc || argv[0][0] == '-') {
				usage();
			}
			output = argv[0];
			argc--;
			argv++;
		} else if (!strcmp(argv[0], "-bpp")) {
			argc--;
			argv++;
			if (!argc || argv[0][0] == '-') {
				usage();
			}
			try {
				tbpp = stoi(argv[0]);
			} catch (std::exception&) {
				usage();
			}
			switch (tbpp) {
				case 0:
				case 1:
				case 2:
				case 4:
				case 8:
				break;
				default:
				usage();
			}
			argc--;
			argv++;
		} else if (!strcmp(argv[0], "-significant")) {
			argc--;
			argv++;
			if (!argc || argv[0][0] == '-') {
				usage();
			}
			try {
				tcolorcount = stoi(argv[0]);
			} catch (std::exception&) {
				usage();
			}
			if (tcolorcount > 256) {
				usage();
			}
			argc--;
			argv++;
		} else if (!strcmp(argv[0], "-resize")) {
			argc--;
			argv++;
			if (!argc || argv[0][0] == '-') {
				usage();
			}
			try {
				tw = stoi(argv[0]);
			} catch (std::exception&) {
				usage();
			}
			argc--;
			argv++;
			if (!argc || argv[0][0] == '-') {
				usage();
			}
			try {
				th = stoi(argv[0]);
			} catch (std::exception&) {
				usage();
			}
			argc--;
			argv++;
		} else if (!strcmp(argv[0], "-border")) {
			argc--;
			argv++;
			border_set = true;
			if (!argc || argv[0][0] == '-') {
				usage();
			}
			try {
				br = stoi(argv[0]);
			} catch (std::exception&) {
				usage();
			}
			argc--;
			argv++;
			if (!argc || argv[0][0] == '-') {
				usage();
			}
			try {
				bg = stoi(argv[0]);
			} catch (std::exception&) {
				usage();
			}
			argc--;
			argv++;
			if (!argc || argv[0][0] == '-') {
				usage();
			}
			try {
				bb = stoi(argv[0]);
			} catch (std::exception&) {
				usage();
			}
			argc--;
			argv++;
			if (br > 15 || bg > 15 || bb > 15) {
				printf("Border RGB values must be in the range of 0-15.\n");
				usage();
			}
		} else if (!strcmp(argv[0], "-dither")) {
			dither = true;
			argc--;
			argv++;
		} else if (!strcmp(argv[0], "-reverse")) {
			reverse = true;
			argc--;
			argv++;
		} else if (!strcmp(argv[0], "-compress")) {
			compress = true;
			argc--;
			argv++;
		} else if (!strcmp(argv[0], "-probe")) {
			probe_only = true;
			argc--;
			argv++;
		} else if (!strcmp(argv[0], "-help")) {
			usage();
		} else if (!strcmp(argv[0], "-palette-file")) {
			argc--;
			argv++;
			if (!argc || argv[0][0] == '-') {
				usage();
			}
			palette_file = argv[0];
			argc--;
			argv++;
		} else if (!strcmp(argv[0], "-debug")) {
			argc--;
			argv++;
			if (!argc || argv[0][0] == '-') {
				usage();
			}
			bool flag_enable = true;
			for (size_t i = 0; i < strlen(argv[0]); i++) {
				switch (argv[0][i]) {
					case '!': {
						flag_enable = false;
					} break;
					case 'p': {
						BitmapX16::set_debug_flag(DebugShowPalette, flag_enable);
					} break;
					case 'c': {
						BitmapX16::set_debug_flag(DebugShowCloseness, flag_enable);
					} break;
					default: {
						printf("Error: Invalid debugging flag.\n");
						usage();
					} break;
				}
				if (argv[0][i] != '!') {
					flag_enable = true;
				}
			}
			argc--;
			argv++;
		} else if (strlen(argv[0]) == 0) {
			// Skip empty arguments.
			argc--;
			argv++;
		} else {
			printf("Error: Invalid command line argument: '%s'\n", argv[0]);
			usage();
		}
	}
	if (input == NULL || ((output != NULL) == probe_only)) {
		if (probe_only) {
			printf("Input must be specified and output must not be!\n");
		} else {
			printf("Input and output must be specified!\n");
		}
		usage();
	}
	if (tbpp == 0) tbpp = 8;
	if (tcolorcount == 0) tcolorcount = (1 << tbpp);
	if (probe_only) {
		tw = 0;
		th = 0;
		tbpp = 0;
		tcolorcount = 0;
	}
	try {
		BitmapX16 bitmap;
		bitmap.enable_compression(compress);
		if (reverse) {
			printf("Loading X16 bitmap file '%s' to be convertedto a PC format...\n", input);
			bitmap.load_x16(input);
			printf("Image has %u significant colors starting at %u and is %u bpp\n", bitmap.get_significant(), bitmap.get_significant_start(), bitmap.get_bpp());
		} else {
			printf("Loading PC image file '%s' to be converted to the X16 bitmap format...\n", input);
			printf("Using at most %u colors (excluding border color) at %u bpp\n", tcolorcount, tbpp);
			if (palette_file != NULL) {
				printf("Using palette file '%s'\n", palette_file);
			} else {
				printf("Using generated palette.\n");
			}
			if (bitmap.compression_enabled()) printf("Compression enabled\n");
			bitmap.load_pc(input);
		}
		if (tw != 0 && th != 0) {
			error_msg_part = "resize the image";
			bitmap.queue_resize(tw, th);
		}
		if (probe_only) {
			error_msg_part = "obtain information";
			tbpp = bitmap.get_bpp();
			uint8_t border = bitmap.get_border_color();
			uint16_t w, h;
			w = bitmap.get_width();
			h = bitmap.get_height();
			uint8_t entry_count = bitmap.get_significant();
			uint8_t entry_start = bitmap.get_significant_start();
			printf("Image BPP: %u\n", tbpp);
			printf("Image size: (%u, %u)\n", w, h);
			printf("Palette starts at %u, has %u entries, and ends at %u\n", entry_start, entry_count == 0 ? 256 : entry_count, entry_start + entry_count);
			printf("Palette:\n");
			vector<PaletteEntry> entries = bitmap.get_palette();
			for (size_t i = 0; i < entries.size(); i+=8) {
				for (size_t j = 0; j < 8; j++) {
					if (i + j > entries.size()) {
						break;
					}
					printf("[%u] = %s, ", i + j, entries[i + j].to_string().c_str());
				}
				printf("\n");
			}
			return 0;
		}
		if (reverse) {
			error_msg_part = "write the file";
			printf("Writing PC image file...\n");
			bitmap.write_pc(output);
		} else {
			error_msg_part = "apply the settings";
			printf("Applying settings...\n");
			if (palette_file != NULL) {
				bitmap.read_palette(palette_file);
			}
			bitmap.enable_dithering(dither);
			bitmap.set_bpp(tbpp);
			bitmap.set_significant(tcolorcount);
			if (border_set) {
				PaletteEntry entry(br, bg, bb);
				bitmap.set_border_color(bitmap.add_palette_entry(entry));
			}
			bitmap.apply();
			uint8_t significant_start = bitmap.get_significant_start();
			uint8_t significant_count = bitmap.get_significant();
			uint8_t significant_end = significant_start + significant_count;
			printf("Significant colors start at %u and end at %u (%u entries)\n", significant_start, significant_end, significant_count);
			printf("Writing X16 bitmap file...\n");
			error_msg_part = "write the file";
			bitmap.write_x16(output);
		}
	} catch (std::exception &e) {
		printf("Failed to %s!\n", error_msg_part);
		return 1;
	}
    return 0;
}
