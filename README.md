# WARNING: This is no longer the home of this repo.
Go [here](https://complecwaft.com/catmeow/bmxconverter) for the new location.
# B16Converter
A converter to/from the B16/BMX bitmap format being developed for the Commander X16

## Building
This program requires Magick++ (which may come with ImageMagick)

Steps:
1. Clone the repo. For example: ``git clone https://github.com/catmeow72/b16converter.git b16converter``
2. cd into the repo directory. For example: ``cd b16converter``
3. Run ``meson setup builddir``
4. Run ``meson compile -C builddir``
5. The binary is ./builddir/b16converter(.exe) where (.exe) is .exe if on Windows, and nothing on other operating systems.

## Usage
Run the binary with the -in option specifying the input file and the -out option specifying the desired output file.

Use -reverse to convert B16 to a PC graphics format

Check the flags with -help

## Testing
Use the test.sh script from an environment with Bash available to automatically convert PACK.png and TEST.png to B16 at various resolutions and bitdepths, with and without dithering. It also converts back to PNG from the B16 files to test that functionality.

Once you have .b16 files, you can test it with my B16 viewer inside the X16 emulator.
