#pragma once
#include <stdint.h>
#include <Magick++.h>
using namespace Magick;
class PaletteEntry {
	public:
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint16_t hash();
	void write(uint8_t *ptr);
	ColorRGB toColor();
	PaletteEntry();
	PaletteEntry(uint8_t *ptr);
	PaletteEntry(uint8_t r, uint8_t g, uint8_t b);
	PaletteEntry(const ColorRGB &color);
};