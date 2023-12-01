#include "bitmapx16.h"
#include <lib.h>
#include <string.h>
#include <fstream>
#include <filesystem>
#include <exception>
using namespace std::filesystem;
#define X16_IMG_START (512+32)
BitmapX16DebugFlags BitmapX16::debug = DebugNone;
float BitmapX16::closeness_to_color(PaletteEntry a, PaletteEntry b) {
	float closeness = ((float)((((float)a.r - (float)b.r) * (1 << 4)) + (((float)a.g - (float)b.g) * (1 << 8)) + ((float)a.b - (float)b.b)));
	if (closeness < 0.0f) {
		closeness = -closeness;
	}
	return closeness;
}
void BitmapX16::set_bpp(uint8_t bpp) {
	this->bpp = bpp;
	quantize_colors = true;
}
uint8_t BitmapX16::get_bpp() const {
	return bpp;
}
void BitmapX16::set_significant(uint8_t value) {
	if (value >= (1 << bpp)) {
		value = (1 << bpp) - 1;
	}
	significant_count = value;
	quantize_colors = true;
}
uint8_t BitmapX16::get_significant() const {
	return significant_count;
}
size_t BitmapX16::get_width() const {
	return w;
}
size_t BitmapX16::get_height() const {
	return h;
}
void BitmapX16::enable_dithering(bool enabled) {
	dither = enabled;
}
bool BitmapX16::dithering_enabled() const {
	return dither;
}
void BitmapX16::resize(size_t w, size_t h) {
	printf("Resizing image to: (%lu, %lu)\n", w, h);
	image->resize(Geometry(w, h));
}
void BitmapX16::queue_resize(size_t w, size_t h) {
	tw = w;
	th = h;
}
void BitmapX16::apply() {
	if (tw != 0 && th != 0) {
		resize(tw, th);
		tw = 0;
		th = 0;
	}
	if (bpp == 0) {
		set_bpp(8);
	}
	if (significant_count == 0 || significant_count >= (1 << bpp)) {
		set_significant((1 << bpp) - 1);
	}
	image->quantizeColors(significant_count);
	image->quantizeDither(dither);
	if (dither) {
		image->quantizeDitherMethod(Magick::FloydSteinbergDitherMethod);
	}
	image->quantize();
	generate_palette();
}
uint8_t BitmapX16::extra_to_real_palette(uint8_t idx) {
	return image_palette_count + idx;
}
void BitmapX16::write_x16(const char *filename) {
	vector<uint8_t> buf;
	size_t bufsize;
	uint8_t pixels_per_byte;
	vector<uint8_t> pixels;
	vector<uint8_t> outpixels;
	size_t outpixelsize;
	size_t pixelCount;
	pixels_per_byte = (8/bpp);
	apply();
	w = image->columns();
	h = image->rows();
	printf("Image size: (%lu, %lu)\n", w, h);
	pixelCount = w * h * 3;
	pixels.resize(pixelCount);
	bufsize = palette_entries.size()*2+32;
	buf.resize(bufsize);
	outpixelsize = ((w*h)/pixels_per_byte);
	outpixels.resize(outpixelsize);
	memset(buf.data(), 0, bufsize);
	buf[0] = 0x42;
	buf[1] = 0x4D;
	buf[2] = 0x58;
	buf[3] = 1; // Version
	buf[4] = bpp;
	
	switch (bpp) {
		case 1:
		buf[5] = 0;
		break;
		case 2:
		buf[5] = 1;
		break;
		case 4:
		buf[5] = 2;
		break;
		case 8:
		buf[5] = 3;
		break;
		default:
		printf("Error: Invalid bit depth.\n");
		throw std::exception();
		break;
	}
	buf[6] = w;
	buf[7] = w >> 8;
	buf[8] = h;
	buf[9] = h >> 8;
	buf[10] = palette_entries.size();
	buf[11] = significant_start;
	uint16_t image_start = 32+(2*palette_entries.size());
	buf[12] = image_start;
	buf[13] = image_start>>8;
	buf[14] = compress ? 255 : 0;
	buf[15] = extra_to_real_palette(border);
	for (size_t i = 16; i < 32; i++) {
		buf[i] = 0; // Reserved bytes.
	}
	for (size_t i = 0; i < palette_entries.size(); i++) {
		palette_entries[i].write(buf.data() + (32+(i*2)));
	}
	for (size_t i = 0, x = 0, y = 0; i < (w * h); i++, x++) {
		if (x >= w) {
			x -= w;
			y += 1;
		}
		ColorRGB px = image->pixelColor(x, y);
		size_t pixelIdx = get_pixel_idx(x, y);
		size_t imagebyteidx = get_byte_idx(pixelIdx);
		uint8_t pixelinbyte = get_inner_idx(pixelIdx);
		outpixels[imagebyteidx] |= (color_to_palette_entry(px) & get_bitmask()) << (bpp * pixelinbyte);
	}
	bufsize += outpixelsize;
	buf.resize(bufsize);
	if (compress) {
		size_t compressed_size = lzsa_compress_inmem(outpixels.data(), buf.data() + image_start, outpixelsize, bufsize - image_start, LZSA_FLAG_RAW_BLOCK, 1, 2);
		if (compressed_size == (size_t)-1) {
			printf("Error compressing file\n");
			throw std::exception();
		}
		bufsize -= outpixelsize - compressed_size;
		buf.resize(bufsize);
	} else {
		memcpy(buf.data() + image_start, outpixels.data(), outpixelsize);
	}
	printf("Writing output file %s...\n", filename);
	std::ofstream outfile(filename,std::ofstream::binary);
	outfile.write((const char*)buf.data(), bufsize);
	outfile.close();
	loaded = true;
}
void BitmapX16::load_x16(const char *filename) {
	vector<uint8_t> buf;
	size_t bufsize = 0;
	size_t bufpos = 0;
	uint16_t palette_used = 0;
	uint8_t pixels_per_byte;
	uint16_t image_start = 0;
	bool compressed = false;
	vector<uint8_t> decompression_buffer;
	vector<uint8_t> pixels;
	bufsize = 3;
	buf.resize(bufsize);
	if (!exists(filename)) {
		printf("File not found!\n");
		throw std::exception();
	}
	std::ifstream infile(filename, std::ifstream::binary);
	if (infile.bad()) {
		printf("Failed to open file!\n");
		throw std::exception();
	}
	infile.read((char*)buf.data() + bufpos, bufsize - bufpos);
	bufpos += bufsize - bufpos;
	uint8_t magic[3] = {0x42, 0x4D, 0x58};
	for (uint8_t i = 0; i < 3; i++) {
		if (buf[i] != magic[i]) {
			printf("Error: Invalid magic bytes.\n");
			throw std::exception();
		}
	}
	bufsize += 12;
	buf.resize(bufsize);
	infile.read((char*)buf.data() + bufpos, bufsize - bufpos);
	bufpos += bufsize - bufpos;
	if (buf[3] > 1) {
		printf("X16 bitmap version %u is unsupported!\n", buf[4]);
		throw std::exception();
	}
	bpp = buf[4];
	/*uint8_t vera_color_depth = buf[5];*/ // Ignore for now.
	pixels_per_byte = (8 / bpp);
	w = buf[6] | (buf[7] << 8);
	h = buf[8] | (buf[9] << 8);
	printf("Image size: (%lu, %lu)\n", w, h);
	palette_used = buf[10];
	if (palette_used == 0) palette_used = 256;
	significant_start = buf[11];
	significant_count = palette_used;
	image_palette_count = 0;
	image_start = buf[12] | (buf[13] << 8);
	if ((int8_t)buf[14] == -1) {
		compressed = true;
	}
	border = buf[15];
	bufsize = std::filesystem::file_size(filename);
	buf.resize(bufsize);
	infile.read((char*)buf.data() + bufpos, bufsize - bufpos);
	bufpos += bufsize - bufpos;
	palette_entries.clear();
	for (size_t i = 0; i < palette_used; i++) {
		palette_entries.push_back(PaletteEntry(buf.data() + (32+(i*2))));
	}
	// Get pixel vector for later use as image data.
	pixels.resize(w * h * 3);
	decompression_buffer.resize(w*h/pixels_per_byte);
	if (compressed) {
		int version;
		size_t bytes = lzsa_decompress_inmem(buf.data() + image_start, decompression_buffer.data(), bufsize - image_start, decompression_buffer.size(), LZSA_FLAG_RAW_BLOCK, &version);
		if (bytes == (size_t)-1) {
			printf("Error decompressing file!\n");
			throw std::exception();
		}
	} else {
		memcpy(decompression_buffer.data(), buf.data() + image_start, decompression_buffer.size());
	}
	size_t outpixelidx = 0;
	for (size_t i = 0, x = 0, y = 0; i < (w * h); i++, x++) {
		// Make sure Y is incremented when necessary.
		if (x >= w) {
			x -= w;
			y += 1;
		}
		// Get the required data.
		size_t pixelIdx = get_pixel_idx(x, y);
		size_t imagebyteidx = get_byte_idx(pixelIdx);
		uint8_t pixelinbyte = get_inner_idx(pixelIdx);
		uint8_t paletteidx = (decompression_buffer[imagebyteidx] >> (pixelinbyte * bpp)) & (get_bitmask());
		PaletteEntry entry = palette_entries[paletteidx];
		uint8_t r = entry.r << 4, g = entry.g << 4, b = entry.b << 4;
		// Add the pixel data to the pixels array.
		pixels[outpixelidx++] = r;
		pixels[outpixelidx++] = g;
		pixels[outpixelidx++] = b;
		if (paletteidx > image_palette_count+significant_start) {
			image_palette_count = paletteidx-significant_start;
		}
	}
	// Create the Magick++ image
	image = new Image(w, h, "RGB", CharPixel, pixels.data());
	// Clean up and set the loaded flag.
	infile.close();
	loaded = true;
}
void BitmapX16::write_pc(const char *filename) {
	if (!loaded) {
		printf("Error: Attempt to write unloaded file!\n");
		throw std::exception();
	}
	image->write(filename);
}
void BitmapX16::load_pc(const char *filename) {
	image = new Image(filename);
	w = image->columns();
	h = image->rows();
	if (bpp == 0) set_bpp(8);
	if (significant_count == 0) set_significant(1 << bpp);
	apply();
	loaded = true;
}
size_t BitmapX16::get_pixel_idx(size_t x, size_t y) {
	return (y * w) + x;
}
size_t BitmapX16::get_byte_idx(size_t pixelidx) {
	return pixelidx / (8/bpp);
}
uint8_t BitmapX16::get_inner_idx(size_t pixelidx) {
	return pixelidx % (8/bpp);
}
uint8_t BitmapX16::get_bitmask() {
	if (bitmask_bpp != bpp) {
		bitmask = (1 << bpp) - 1;
	}
	return bitmask;
}
uint8_t BitmapX16::get_orable_pixel(uint8_t pixelinbyte, uint8_t color) {
	return (color & get_bitmask()) << (bpp * ((8/bpp) - pixelinbyte - 1));
}

BitmapX16::BitmapX16() {
	palette_entries = vector<PaletteEntry>();
}
void BitmapX16::generate_palette() {
	size_t min;
	uint16_t max;
	if (!generate_palette_enabled || !write_palette) {
		significant_count = write_palette ? palette_entries.size() : 0;
		max = significant_count;
		min = 256 - max;
		if (min >= 16 && write_palette) {
			significant_start = 16;
		}
		if (max > 256) max = 256;
		if (bpp == 0) {
			if (max <= 4) {
				bpp = 2;
			} else if (max <= 16) {
				bpp = 4;
			} else {
				bpp = 8;
			}
		}
		return;
	}
	max = (uint16_t)image->colorMapSize();
	if (max > 256) max = 256;
	if (bpp == 0) {
		if (max <= 4) {
			bpp = 2;
		} else if (max <= 16) {
			bpp = 4;
		} else {
			bpp = 8;
		}
	}
	min = 256 - (1 << bpp);
	if (min >= 16) {
		significant_start = 16;
	}
	if (significant_count == 0) {
		significant_count = max;
	}
	bitmask = (1 << bpp) - 1;
	image_palette_count = max;
	palette_entries.clear();
	for (uint16_t i = 0; i < image_palette_count; i++) {
		ColorRGB map_color = image->colorMap(i);
		palette_entries.push_back(PaletteEntry(map_color));
	}
	for (uint16_t i = 0; i < extra_palette_entries.size(); i++) {
		palette_entries.push_back(extra_palette_entries[i]);
	}
	significant_count = palette_entries.size();
	if (debug & DebugShowPalette) {
		for (size_t i = 0; i < palette_entries.size(); i++) {
			uint8_t significant_end = significant_start+image_palette_count;
			bool significant = i >= significant_start && i < significant_end;
			bool extra = i >= image_palette_count && i < image_palette_count+extra_palette_entries.size();
			printf("palette[%02x] = %s %s\n", (uint16_t)i, palette_entries[i].to_string().c_str(), significant ? "(Significant)" : extra ? "(Extra)" : "");
		}
	}
}
uint8_t BitmapX16::add_palette_entry(PaletteEntry entry) {
	extra_palette_entries.push_back(entry);
	return (uint8_t)(extra_palette_entries.size() - 1);
}
uint8_t BitmapX16::color_to_palette_entry(const ColorRGB &rgb) {
	PaletteEntry color(rgb);
	float closeness = 100000.0f;
	uint8_t output;
	if (debug & DebugShowCloseness) {
		printf("Closest color for %s: ", color.to_string().c_str());
	}
	for (size_t i = 0; i < image_palette_count; i++) {
		float possibility_closeness = closeness_to_color(palette_entries[i], color);
		//printf("Closeness: %f", possibility_closeness);
		if (possibility_closeness < closeness) {
			output = i;
			closeness = possibility_closeness;
		}
	}
	if (debug & DebugShowCloseness) {
		PaletteEntry output_entry = palette_entries[output];
		printf("%s\n", output_entry.to_string().c_str());
	}
	//PaletteEntry entry = palette[output];
	//printf("Color: #%0x%0x%0x -> Palette entry#%0x%0x%0x, closeness: %f\n", color.r, color.g, color.b , entry.r, entry.g, entry.b, closeness);
	return output;
}
void BitmapX16::set_debug_flag(BitmapX16DebugFlags flag, bool enabled) {
	int value = (int)debug;
	if (enabled) {
		value |= (int)flag;
	} else {
		value &= ~(int)flag;
	}
	debug = (BitmapX16DebugFlags)value;
}
BitmapX16::~BitmapX16() {
	if (image != nullptr) {
		delete image;
	}
}
uint8_t BitmapX16::get_significant_start() const {
	return significant_start;
}
uint8_t BitmapX16::get_border_color() const {
	return border;
}
void BitmapX16::set_border_color(uint8_t idx) {
	border = idx;
}
void BitmapX16::enable_compression(bool enabled) {
	compress = enabled;
}
bool BitmapX16::compression_enabled() const {
	return compress;
}
vector<PaletteEntry> BitmapX16::get_palette() const {
	return palette_entries;
}
vector<PaletteEntry> BitmapX16::get_extra_entries() const {
	return extra_palette_entries;
}
void BitmapX16::set_palette(vector<PaletteEntry> entries) {
	palette_entries = entries;
	palette_entries.shrink_to_fit();
	extra_palette_entries.clear();
	generate_palette_enabled = false;
	image->quantizeColors(entries.size());
	image->quantizeColorSpace(MagickCore::RGBColorspace);
	image->colorMapSize(entries.size());
	for (uint16_t i = 0; i < entries.size(); i++) {
		image->colorMap(i, entries[i].toColor());
	}
	image->quantize();
}
void BitmapX16::enable_palette_generation() {
	generate_palette_enabled = true;
}
bool BitmapX16::palette_generation_enabled() const {
	return generate_palette_enabled && write_palette;
}
void BitmapX16::read_palette(const char *filename) {
	size_t fsize = std::filesystem::file_size(filename);
	if (fsize > 512 || (fsize % 2 != 0)) {
		printf("Invalid palette file size! Palette files must be raw VERA palette data, sized as a multiple of 2, and up to 512 bytes(256 entries)");
		throw std::exception();
	}
	vector<PaletteEntry> entries;
	uint8_t entry[2];
	std::ifstream file(filename, std::ifstream::binary|std::ifstream::in);
	for (size_t i = 0; i < fsize; i += 2) {
		file.read((char*)entry, 2);
		entries.push_back(PaletteEntry(&entry[0]));
		if (file.eof()) {
			break;
		}
		if (file.bad()) {
			printf("Error reading file!");
			throw std::exception();
		}
	}
	set_palette(entries);
}