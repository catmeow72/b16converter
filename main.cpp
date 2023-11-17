#include <Magick++.h>
#include <iostream>
#include <fstream>
#include <map>
#include "palette.h"
#include <string.h>
#include <vector>
#include <string>
#include <exception>
#include "bitmapx16.h"
using std::vector;
using std::map;
using std::stoi;
using namespace Magick;
void usage() {
	printf("Usage: veraconvert: [options]\n");
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
	printf("-reverse\n");
	printf("\tConverts to PC formats. Incompatible with -dither, -type, and -significant - they will be ignored.\n");
	printf("-help\n");
	printf("\tDisplays this help message.\n");
	exit(1);
}
int main(int argc, char **argv) {
	const char *input = NULL, *output = NULL;
	size_t tw = 0, th = 0; // Target width & height;
	uint8_t tbpp = 0;
	uint16_t tcolorcount = 0;
	bool dither = false;
	bool reverse = false;
	uint8_t br, bg, bb;
	bool border_set = false;
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
		} else if (!strcmp(argv[0], "-help")) {
			usage();
		} else {
			printf("Error: Invalid command line argument.\n");
			usage();
		}
	}
	if (input == NULL || output == NULL) {
		printf("Input and output must be specified!\n");
		usage();
	}
	if (tbpp == 0) tbpp = 8;
	if (tcolorcount == 0) tcolorcount = (1 << tbpp);
	try {
		BitmapX16 bitmap;
		if (reverse) {
			printf("Converting %s to a PC format...\n", input);
			bitmap.load_x16(input);
		} else {
			printf("Using at most %u colors at %u bpp\n", tcolorcount, tbpp);
			printf("Converting %s to BMX16...\n", input);
			bitmap.load_pc(input);
		}
		if (tw != 0 && th != 0) {
			bitmap.queue_resize(tw, th);
		}
		if (reverse) {
			bitmap.write_pc(output);
		} else {
			bitmap.enable_dithering(dither);
			bitmap.set_bpp(tbpp);
			bitmap.set_significant(tcolorcount);
			if (border_set) {
				PaletteEntry entry(br, bg, bb);
				bitmap.set_border_color(bitmap.add_palette_entry(entry));
			}
			bitmap.apply();
			bitmap.write_x16(output);
		}
	} catch (std::exception &e) {
		printf("Failed to convert image '%s'!\n", input);
	}
    return 0;
}
