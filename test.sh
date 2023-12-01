#!/bin/bash
bool_true() {
	[ "$1" = "true" ] 2>/dev/null || [ "$1" -ne 0 ] 2>/dev/null
	[ $? -ne 0 ] # Convert $? to 1 or 0 value
	return $?
}
bool_false() {
	! bool_true
	return $?
}
cmd_avail() {
	[ -x "$(command -v "$1")" ] && return 0 || return 1
}
tests=0
failed=0
succeeded=0
color=1
tput_avail=0
if cmd_avail tput; then
	tput_avail=1
fi
if bool_true $tput_avail; then
	COLORS=$(tput color 2>/dev/null)
	if [ $? -eq 0 ] && [ $COLORS -gt 2 ]; then
		color=1
	else
		color=0
	fi
fi
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
-R|--no-reverse
	Disables reverse operation testing
-D|--no-dither
	Disables dithering testing
-S|--no-compress
	Disables compression testing
--no-generate-palette
	Disables palette generation when testing and uses prebuilt palette files only
-p|--palette-file
	Uses the specified palette file.
-d|--debug
	Use debug flags with the converter program.
-n|--no-defaults
	Disables default settings
-c|--color
	Enables color.
-C|--no-color
	Disables color.
-h|--help
	Shows this message.

EOF
	exit 1
}
oldpwd="$(pwd)"
cd "$(dirname "$0")"
converter="./builddir/bmxconverter"
prebuilt=0
images=()
bpps=()
resize=()
palettes=()
generate_palette=1
enable_defaults=1
dither=1
reverse=1
enable_compression=1
enable_reverse=1
enable_dither=1
enable_probe=1
debug_flags=""
outdir="testout"
OPTIONS=$(getopt -o "b:hp:i:r:no:d:p:cCDRS" --long "help,use-program:,input-image:,output-bpp:,resize:,no-defaults,output-dir:,debug:,no-reverse,no-dither,no-compress,palette-file,no-generate-palette,no-probe,color,no-color" -- "$@")
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
		-R|--no-reverse)
			enable_reverse=0
			shift
			;;
		-D|--no-dither)
			enable_dither=0
			shift
			;;
		-S|--no-compress)
			enable_compression=0
			shift
			;;
		-d|--debug)
			debug_flags="${debug_flags}$2"
			shift 2
			;;
		-p|--palette-file)
			palettes+="$2"
			shift 2
			;;
		--no-generate-palette)
			generate_palette=0
			shift
			;;
		--no-probe)
			enable_probe=0
			shift
			;;
		-c|--color)
			color=1
			shift
			;;
		-C|--no-color)
			color=0
			shift
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
if [ $generate_palette -ne 0 ]; then
	palettes+=""
fi
if [ $enable_defaults -ne 0 ]; then
	images+=("TEST.png" "PACK.png" "CAT.jpg")
	bpps+=(1 2 4 8)
	resize+=("8x8" "16x16" "32x32" "64x64" "320x240" "640x480")
fi
if [ $prebuilt -eq 0 ]; then
	meson setup builddir
	meson compile -C builddir || exit $?
fi
bold() {
	if bool_false "$color"; then
		return
	fi
	if bool_true "$tput_avail"; then
		tput bold
	else
		printf "\033[1m"
	fi
}
italic() {
	if bool_false "$color"; then
		return
	fi
	if bool_true "$tput_avail"; then
		tput enter_italics_mode
	else
		printf "\033[3m"
	fi
}
setfgcolor() {
	if bool_false "$color"; then
		return
	fi
	if bool_true "$tput_avail"; then
		tput setaf "$1"
	else
		printf "\033[0;3%sm" "$1"
	fi
}
setbgcolor() {
	if bool_false "$color"; then
		return
	fi
	if bool_true "$tput_avail"; then
		tput setab "$1"
	else
		printf "\033[0;4%sm" "$1"
	fi
}
resetcolor() {
	if bool_false "$color"; then
		return
	fi
	if bool_true "$tput_avail"; then
		tput sgr0
	else
		printf "\033[0m"
	fi
}
mkdir -p "$outdir"
run() {
	converter="$1"
	shift
	infile="$1"
	shift
	outfile="$1"
	shift
	reverse="$1"
	shift
	extra_flags_run=()
	if [ -n "$reverse" ]; then
		extra_flags_run+=( "$reverse" )
	fi
	setfgcolor 2
	bold
	printf "Running: %s\n" "$converter $* -in $infile -out $outfile"
	resetcolor
	setfgcolor 8
	tests=$(($tests+1))
	"$converter" -in "$infile" -out "$outfile" "${extra_flags_run[@]}" "$@"
	if [ $? -ne 0 ]; then
		setfgcolor 1
		printf "Test failed.\n"
		resetcolor
		failed=$(($failed+1))
	else
		succeeded=$(($succeeded+1))
	fi
	resetcolor
	if [ $enable_probe -ne 0 ]; then
		setfgcolor 2
		bold
		printf "Probing %s...\n" "$outfile"
		resetcolor
		setfgcolor 8
		"$converter" -in "$outfile" -probe "${extra_flags_run[@]}"
		resetcolor
		tests=$(($tests+1))
		if [ $? -ne 0 ]; then
			setfgcolor 1
			printf "Test failed.\n"
			resetcolor
			failed=$(($failed+1))
		else
			succeeded=$(($succeeded+1))
		fi
	fi
}
for img in "${images[@]}"; do
	for bpp in "${bpps[@]}"; do
		for size in "${resize[@]}"; do
			for compressflag in -compress ""; do
				for palette in "${palettes[@]}"; do 
					width="$(echo -n "$size" | cut -dx -f1)"
					height="$(echo -n "$size" | cut -dx -f2)"
					name="$(basename "$img" | sed 's/\.png$//' | sed 's/\.jpg$//' | sed 's/\.jpeg$//')"
					name="$(printf "%s.%sP.%sB" "$name" "$width" "$bpp")"
					extraflags=()
					if [ -n "$palette" ]; then
						extraflags+=( "-palette-file" "$palette" )
					fi
					if [ -n "$compressflag" ]; then
						if [ $enable_compression -eq 0 ]; then
							continue
						fi
						extraflags+=( "$compressflag" )
						name+=".C"
					fi
					run "$converter" "$img" "$outdir/$name.BMX" "" "${extraflags[@]}" "" -bpp "$bpp" -resize "$width" "$height" -border 15 0 15 -debug "$debug_flags"
					if [ $enable_dither -ne 0 ]; then
						run "$converter" "$img" "$outdir/$name.D.BMX" "" "${extraflags[@]}" "" -bpp "$bpp" -resize "$width" "$height" -dither -border 15 0 15 -debug "$debug_flags"
					fi
					if [ $enable_reverse -ne 0 ]; then
						run "$converter" "$outdir/$name.BMX" "$outdir/$name.PNG" -reverse "${extraflags[@]}" -resize "$width" "$height" -debug "$debug_flags"
						if [ $enable_dither -ne 0 ]; then
							run "$converter" "$outdir/$name.D.BMX" "$outdir/$name.D.PNG" -reverse "${extraflags[@]}" -resize "$width" "$height" -dither -debug "$debug_flags"
						fi
					fi
				done
			done
		done
	done
done
printf "%s total test cases, %s failed, %s succeeded, %s%% succeeded and %s%% failed.\n" "$tests" "$failed" "$succeeded" "$((($succeeded*100)/$tests))" "$((($failed*100)/$tests))"
cd "$oldpwd"
