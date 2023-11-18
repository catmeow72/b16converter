#!/bin/bash
usage() {
	cat << EOF
$0 usage:
-p|--use-program <program path>
	Uses the specified already-compiled binary rather than compiling manually
-i|--input-image <image path>
	Adds an image input at the specified path to the image list.
-b|--output-bpp <bpp>
	Adds a bitdepth to the list to test with
-r|--resize <width>x<height>
	Adds a resize to the list to test with
--no-reverse
	Disables reverse operation testing
--no-dither
	Disables dithering testing
-d|--debug
	Use debug flags with the converter program.
-n|--no-defaults
	Disables default settings
EOF
	exit 1
}
oldpwd="$(pwd)"
cd "$(dirname "$0")"
converter="./builddir/b16converter"
prebuilt=0
images=()
bpps=()
resize=()
enable_defaults=1
dither=1
reverse=1
enable_reverse=1
enable_dither=1
debug_flags=""
outdir="testout"
OPTIONS=$(getopt -o "b:hp:i:r:no:d:" --long "help,use-program:,input-image:,output-bpp:,resize:,no-defaults,output-dir:,debug:,no-reverse,no-dither" -- "$@")
if [ $? != 0 ]; then
	echo "Getopt error."
	usage
fi
eval set -- "$OPTIONS"
while [ -n "$1" ]; do
	case "$1" in
		-o|--output-dir)
			outdir="$2"
			shift 2
			;;
		-h|--help)
			usage
			;;
		-p|--use-program)
			converter="$2"
			prebuilt=1
			shift 2
			;;
		-i|--input-image)
			images+="$2"
			shift 2
			;;
		-b|--output-bpp)
			bpp+="$2"
			shift 2
			;;
		-r|--resize)
			resize+="$2"
			shift 2
			;;
		-n|--no-defaults)
			enable_defaults=0
			shift
			;;
		--no-reverse)
			enable_reverse=0
			shift
			;;
		--no-dither)
			enable_dither=0
			shift
			;;
		-d|--debug)
			debug_flags="${debug_flags}$2"
			shift 2
			;;
		--)
			shift
			break
			;;
		*)	
			echo "Invalid option: $0"
			usage
			;;
	esac
done
if [ $enable_defaults -ne 0 ]; then
	images+=("TEST.png" "PACK.png")
	bpps+=(1 2 4 8)
	resize+=("8x8" "16x16" "32x32" "64x64" "320x240" "640x480")
fi
if [ $prebuilt -eq 0 ]; then
	meson setup builddir
	meson compile -C builddir || exit $?
fi
mkdir -p "$outdir"
run() {
	printf "Running: %s\n" "$*"
	"$@"
}
for img in "${images[@]}"; do
	for bpp in "${bpps[@]}"; do
		for size in "${resize[@]}"; do
			width="$(echo -n "$size" | cut -dx -f1)"
			height="$(echo -n "$size" | cut -dx -f2)"
			name="$(basename "$img" | sed 's/\.png$//')"
			name="$(printf "%s.%sP.%sB" "$name" "$width" "$bpp")"
			run "$converter" -in "$img" -out "$outdir/$name.B16" -bpp "$bpp" -resize "$width" "$height" -border 15 0 15 -debug "$debug_flags"
			if [ $enable_dither -ne 0 ]; then
				run "$converter" -in "$img" -out "$outdir/$name.D.B16" -bpp "$bpp" -resize "$width" "$height" -dither -border 15 0 15 -debug "$debug_flags"
			fi
			if [ $enable_reverse -ne 0 ]; then
				run "$converter" -reverse -in "$outdir/$name.B16" -out "$outdir/$name.PNG" -resize "$width" "$height" -debug "$debug_flags"
				if [ $enable_dither -ne 0 ]; then
					run "$converter" -reverse -in "$outdir/$name.D.B16" -out "$outdir/$name.D.PNG" -resize "$width" "$height" -dither -debug "$debug_flags"
				fi
			fi
		done
	done
done

cd "$oldpwd"
