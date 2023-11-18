#pragma once
#include <stdint.h>
#include <Magick++.h>
#include <string>
using namespace Magick;
class PaletteEntry {
	public:
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint16_t hash() const;
	void write(uint8_t *ptr) const;
	ColorRGB toColor() const;
	std::string to_string() const;
	PaletteEntry();
	PaletteEntry(uint8_t *ptr);
	PaletteEntry(uint8_t r, uint8_t g, uint8_t b);
	PaletteEntry(const ColorRGB &color);
};