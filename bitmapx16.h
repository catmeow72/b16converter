#pragma once
#include <vector>
#include <Magick++.h>
#include "palette.h"
using namespace Magick;
using std::vector;
enum BitmapX16DebugFlags : int {
	DebugNone = 0,
	DebugShowPalette = (1 << 0),
	DebugShowCloseness = (1 << 1)
};
class BitmapX16 {
	/// \brief Bits per pixel of the image
	uint8_t bpp = 0;
	/// \brief The list of palette entries.
	vector<PaletteEntry> palette_entries;
	/// \brief A copy of the extra palette entries.
	vector<PaletteEntry> extra_palette_entries;
	/// \brief The amount of colors used within the palette
	uint8_t significant_count = 0;
	/// \brief The beginning of the colors used.
	uint8_t significant_start = 0;
	/// \brief Imagemagick image to apply modifications to and read/write to an X16 bitmap
	Image *image = nullptr;
	/// \brief Set to true to queue color quantization. Set automatically when needed.
	bool quantize_colors = false;
	/// \brief Enables LZSA compression
	bool compress = false;
	/// \brief False to set the used palette to 0, and palette start also to 0.
	bool write_palette = true;
	/// \brief True if the palette should be generated, false if it has been set manually and shouldn't be regenerated.
	bool generate_palette_enabled = true;
	/// \brief Current width
	size_t w = 0;
	/// \brief Current height
	size_t h = 0;
	/// \brief Target width
	size_t tw = 0;
	/// \brief Target height
	size_t th = 0; 
	/// \brief Whether or not to dither
	bool dither = false;
	/// \brief Whether or not the image has been loaded properly
	bool loaded = false;
	/// \brief The cache of the bitmask to use for pixels
	uint8_t bitmask = 0;
	/// \brief The bits per pixel the bitmap was created with.
	uint8_t bitmask_bpp = 0;
	/// \brief The palette entry corrosponding to the border color
	uint8_t border = 0;
	/// \brief The amount of palette entries the image itself is supposed to have.
	uint8_t image_palette_count = 0;
	/// \brief Generates a palette from the current image
	void generate_palette();
	/// \brief Actually resizes the image. User code should call queue_resize then apply
	/// \param w The width to resize to
	/// \param h The height to resize to
	void resize(size_t w, size_t h); 
	/// \brief Gets the pixel index within this image based on X and Y values, as well as the width of the image
	/// \param x The X value of the pixel
	/// \param y The Y value of the pixel
	/// \returns The pixel index within this image
	size_t get_pixel_idx(size_t x, size_t y);
	/// \brief Returns the index within a byte buffer containing raw VERA image data a pixel is located in based on the pixel index and bpp
	/// \param pixelidx The index of the pixel
	/// \returns The byte index
	size_t get_byte_idx(size_t pixelidx);
	/// \brief Returns the index of a pixel within a byte based on the index of the pixel.
	/// \param pixelidx The index of the pixel
	/// \returns The pixel index within a byte
	uint8_t get_inner_idx(size_t pixelidx);
	/// \brief Returns the correct bitmask for a pixel. May be calculated if the cache is invalid.
	/// \returns A bitmask for any first pixel in a byte in the image. Can be shifted to get different pixels within a byte.
	uint8_t get_bitmask();
	/// \brief Gets a value that can be OR'ed to a byte in a buffer based on the pixel index, the BPP of the image, and the value to set.
	/// \param inner_idx The index of the pixel within the byte
	/// \param pixel The value of the pixel
	/// \returns A value that can be OR'ed with a byte in a buffer that doesn't have a pixel at the specified index.
	uint8_t get_orable_pixel(uint8_t inner_idx, uint8_t pixel);
	/// \brief Converts a color to the nearest palette entry
	/// \param rgb The color to convert
	/// \returns The palette entry index
	uint8_t color_to_palette_entry(const ColorRGB &rgb);
	uint8_t extra_to_real_palette(uint8_t idx);
	float closeness_to_color(PaletteEntry a, PaletteEntry b);
	public:
	vector<PaletteEntry> get_palette() const;
	vector<PaletteEntry> get_extra_entries() const;
	void read_palette(const char *filename);
	/// \brief Sets the palette to use
	/// \param entries The entries to replace the existing palette
	void set_palette(vector<PaletteEntry> entries);
	/// \brief Enables palette generation after disabling it with set_palette.
	void enable_palette_generation();
	/// \brief Checks the status of palette generation
	/// \returns true if palette generation is enabled, false otherwise.
	bool palette_generation_enabled() const;
	/// \brief Sets the border color extra palette entry.
	/// \param idx The index of the palette entry, must be an index into the extra palette entry list.
	void set_border_color(uint8_t idx);
	/// \brief Gets the current extra palette entry of the border color
	/// \returns The border color palette entry from the extra palette entry list
	uint8_t get_border_color() const;
	/// \brief Adds an entry to the list of extra palette entries. Not guaranteed to continue existing after a new palette entry has been added.
	/// \param entry The new entry to add
	/// \returns The palette index within the extra palette entries of the new entry
	uint8_t add_palette_entry(PaletteEntry entry);
	/// \brief Sets the bits per pixel of the image
	/// \param bpp The bits per pixel of the image, one of 1, 2, 4, or 8
	void set_bpp(uint8_t bpp = 8);
	/// \brief Returns the bits per pixel of the image
	/// \returns The bits per pixel of the image, one of 1, 2, 4, or 8
	uint8_t get_bpp() const;
	/// \brief Sets the maximum amount of colors to be used.
	/// \param value The maximum amount of colors to use.
	void set_significant(uint8_t value = 0);
	/// \brief Returns the maximum amount of colors to be used
	/// \returns The maximum amount of colors once written
	uint8_t get_significant() const;
	/// \brief Gets the beginning index of the significant palette entries.
	/// \returns The start index of significant palette entries.
	uint8_t get_significant_start() const;
	/// \brief Queues a resize operation. Call BitmapX16::apply() to apply
	/// \param w The width to resize to
	/// \param h The height to resize to
	void queue_resize(size_t w, size_t h);
	/// \brief Gets the width of the image
	/// \returns The width of the image
	size_t get_width() const;
	/// \brief Gets the height of the image
	/// \returns The height of the image
	size_t get_height() const;
	/// \brief Enables or disables dithering
	/// \param enabled Pass true to enable, false to disable
	void enable_dithering(bool enabled = true);
	/// \brief Returns the status of the dithering flag
	/// \returns The value of the dithering flag
	bool dithering_enabled() const;
	/// \brief Enables or disables LZSA compression
	/// \param enabled Pass true to enable, false to disable
	void enable_compression(bool enabled);
	/// \brief Returns the status of the LZSA compression flag
	/// \returns The value of the compression flag.
	bool compression_enabled() const;
	/// \brief Applies queued operations to the internal representation of the image
	void apply();
	/// \brief Applies queued operations and writes the image to a PC-compatible file
	/// \param filename The path to the file to write
	void write_pc(const char *filename);
	/// \brief Applies queued operations and wri the image to a Commander X16-compatible file
	/// \param filename The path to the file to write
	void write_x16(const char *filename);
	/// \brief Loads a PC-compatible image file
	/// \param filename The path to the file to load
	void load_pc(const char *filename);
	/// \brief Loads a Commander X16-compatible image file
	/// \param filename The path to the file to load
	void load_x16(const char *filename);
	static void set_debug_flag(BitmapX16DebugFlags flag, bool enabled = true);
	/// \brief The debug flags to use.
	static BitmapX16DebugFlags debug;
	/// \brief Constructs an unloaded X16-compatible bitmap image. Call load_pc or load_x16 before using any other functions.
	BitmapX16();
	~BitmapX16();
};