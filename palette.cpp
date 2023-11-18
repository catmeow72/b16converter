#include "palette.h"
ColorRGB PaletteEntry::toColor() const {
	ColorRGB color;
	color.red(((double)r) / 15.0);
	color.green(((double)g) / 15.0);
	color.blue(((double)b) / 15.0);
	return color;
}
void PaletteEntry::write(uint8_t *ptr) const {
	ptr[0] = ((g & 0b1111) << 4) | (b & 0b1111);
	ptr[1] = r;
}
std::string PaletteEntry::to_string() const {
	char buf[65];
	snprintf(buf, 64, "#%0x%0x%0x", r, g, b);
	std::string output(buf);
	return output;
}
PaletteEntry::PaletteEntry(uint8_t *ptr) {
	b = (ptr[0] & 0b1111);
	g = ((ptr[0] >> 4) & 0b1111);
	r = ptr[1] & 0b1111;
}
PaletteEntry::PaletteEntry(uint8_t r, uint8_t g, uint8_t b) {
	this->r = r;
	this->g = g;
	this->b = b;
}
PaletteEntry::PaletteEntry(const ColorRGB &rgb) {
	r = (uint8_t)((rgb.red()) * 15);
	g = (uint8_t)((rgb.green()) * 15);
	b = (uint8_t)((rgb.blue()) * 15);
}
uint16_t PaletteEntry::hash() const {
	return ((r & 0b1111) << 8) | ((g & 0b1111) << 4) | (b & 0b1111);
}
PaletteEntry::PaletteEntry() {
	r = 0;
	g = 0;
	b = 0;
}