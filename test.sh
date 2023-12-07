#!/bin/bash
BLACK=0
RED=1
GREEN=2
YELLOW=3
BLUE=4
MAGENTA=5
CYAN=6
WHITE=7
GRAY=8
DEFAULT=9
bool_true() {
	[ "$1" = "true" ] 2>/dev/null || [ "$1" -ne 0 ] 2>/dev/null
	[ $? -eq 0 ] # Convert $? to 1 or 0 value
	return $?
}
bool_false() {
	! bool_true "$1"
	return $?
}
cmd_avail() {
	[ -x "$(command -v "$1")" ] && return 0 || return 1
}
tests=0
failed=0
succeeded=0
failed_ids=()
color=1
tput_avail=0
if cmd_avail tput; then
	tput_avail=1
fi
if bool_true $tput_avail; then
	COLORS=$(tput colors 2>/dev/null)
	if [ $? -eq 0 ] && [ $COLORS -gt 2 ]; then
		color=1
		if [ $COLORS -lt 8 ]; then
			GRAY=9
		fi
	else
		color=0
	fi
fi
usage() {
	cat << EOF
$0 usage:
-o|--output-dir <output directory> 
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
-O|--reverse-dir <directory for reverse conversion files>
	Sets the directory for files that have been converted to PNG from BMX
	No effect when reverse conversion is disabled
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
reverse_dir_set=0
reversedir=""
OPTIONS=$(getopt -o "b:hp:i:r:no:d:p:cCDRSO" --long "help,use-program:,input-image:,output-bpp:,resize:,no-defaults,output-dir:,debug:,no-reverse,no-dither,no-compress,palette-file,no-generate-palette,no-probe,color,no-color,reverse-dir" -- "$@")
if [ $? != 0 ]; then
	echo "Getopt error."
	usage
fi
eval set -- "$OPTIONS"
while [ -n "$1" ]; do
	case "$1" in
		-O|--reverse-dir)
			reversedir="$2"
			reverse_dir_set=1
			shift 2
			;;
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
if bool_false $reverse_dir_set; then
	reversedir="$outdir/reverse"
fi
if bool_true $generate_palette; then
	palettes+=""
fi
if bool_true $enable_defaults; then
	images+=("TEST.png" "PACK.png" "CAT.jpg")
	bpps+=(1 2 4 8)
	resize+=("64x64" "320x240" "640x480")
	palettes+=("DPAL.BIN")
fi
if bool_true $enable_reverse; then
	mkdir -p $reversedir
fi
if bool_false $prebuilt; then
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
		printf "\033[38;5;%sm" "$1"
	fi
}
setbgcolor() {
	if bool_false "$color"; then
		return
	fi
	if bool_true "$tput_avail"; then
		tput setab "$1"
	else
		printf "\033[48;5;%sm" "$1"
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
extension() {
	len="$(printf "%s" "$1" | sed 's/[^.]*\.[^.]*/./g' | wc -c)"
	printf "%s" "$1" | cut -d. -f$(($len+1))
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
	infile_for_id=$(basename $infile)
	outfile_for_id=$(basename $outfile)
	extra_flags_run=()
	extra_flags_probe=()
	if bool_true "$reverse"; then
		extra_flags_run+=( "-reverse" )
	else
		extra_flags_probe+=( "-reverse" )
	fi
	id="convert_${infile_for_id}_${outfile_for_id}"
	if [ -n "$reverse" ]; then
		id="${id}_reverse"
	fi
	setfgcolor $WHITE
	bold
	printf "Running test %s\n" "$id"
	printf "Command: %s\n" "$converter -in $infile -out $outfile ${extra_flags_run[*]} $*"
	resetcolor
	setfgcolor $GRAY
	tests=$(($tests+1))
	"$converter" -in "$infile" -out "$outfile" "${extra_flags_run[@]}" "$@"
	if [ $? -ne 0 ]; then
		setfgcolor 1
		printf "Test $id failed.\n"
		resetcolor
		failed=$(($failed+1))
		failed_ids+=($id)
	else
		setfgcolor $GREEN
		printf "Test $id succeeded!\n"
		resetcolor
		succeeded=$(($succeeded+1))
	fi
	resetcolor
	if [ $enable_probe -ne 0 ]; then
		id="probe_${outfile_for_id}"
		if [ -z "$reverse" ]; then
			id="${id}_pc"
		fi
		setfgcolor $BLUE
		bold
		printf "Running test %s\n" "$id"
		printf "Probing %s...\n" "$outfile"
		resetcolor
		setfgcolor $GRAY
		printf "Command: %s\n" "$converter -in $infile -probe ${extra_flags_probe[*]}"
		"$converter" -in "$outfile" -probe "${extra_flags_probe[@]}"
		resetcolor
		tests=$(($tests+1))
		if [ $? -ne 0 ]; then
			setfgcolor $RED
			printf "Test $id failed.\n"
			resetcolor
			failed=$(($failed+1))
			failed_ids+=($id)
		else
			setfgcolor $GREEN
			printf "Test $id succeeded!\n"
			resetcolor
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
					ext="$(extension "$img")"
					ext_upper="$(printf "%s" "$ext" | tr '[:lower:]' '[:upper:]')"
					name="$(basename -s ".$ext" "$img")"
					name="$(printf "%s.%s.%sPX.%sB" "$name" "$ext_upper" "$width" "$bpp")"
					extraflags=()
					if [ -n "$palette" ]; then
						extraflags+=( "-palette-file" "$palette" )
						name+=".PL-$(basename -s ".$(extension "$palette")" "$palette" | tr '[:lower:]' '[:upper]')"
					fi
					if [ -n "$compressflag" ]; then
						if [ $enable_compression -eq 0 ]; then
							continue
						fi
						extraflags+=( "$compressflag" )
						name+=".C"
					fi
					run "$converter" "$img" "$outdir/$name.BMX" false "${extraflags[@]}" -bpp "$bpp" -resize "$width" "$height" -border 15 0 15 -debug "$debug_flags"
					if bool_true $enable_dither; then
						run "$converter" "$img" "$outdir/$name.D.BMX" false "${extraflags[@]}" -bpp "$bpp" -resize "$width" "$height" -dither -border 15 0 15 -debug "$debug_flags"
					fi
					if bool_true $enable_reverse; then
						run "$converter" "$outdir/$name.BMX" "$reversedir/$name.PNG" true "${extraflags[@]}" -resize "$width" "$height" -debug "$debug_flags"
						if bool_true $enable_dither; then
							run "$converter" "$outdir/$name.D.BMX" "$reversedir/$name.D.PNG" true "${extraflags[@]}" -resize "$width" "$height" -dither -debug "$debug_flags"
						fi
					fi
				done
			done
		done
	done
done
badcolor=$RED
goodcolor=$GREEN
okcolor=$YELLOW
success_good=90
success_ok=50
percent_out_of_tests() {
	if [ "$tests" -eq 0 ]; then
		printf "100"
	else
		printf "%s" "$((($1*100)/$tests))"
	fi
}
invert_percent() {
	printf "%s" "$((100-$1))"
}
success_percent="$(percent_out_of_tests $succeeded)"
failed_percent="$(percent_out_of_tests $failed)"
set_color_by_good_percentage() {
	if [ "$1" -ge $success_good ]; then
		setfgcolor "$goodcolor"
	elif [ "$1" -ge $success_ok ]; then
		setfgcolor "$okcolor"
	else
		setfgcolor "$badcolor"
	fi
}
set_color_by_bad_percentage() {
	set_color_by_good_percentage "$(invert_percent $1)"
}
bold
setfgcolor $WHITE
printf "%s total test cases, " "$tests"
set_color_by_bad_percentage "$failed_percent"
printf "%s failed (%s%%) " "$failed" "$failed_percent"
setfgcolor $WHITE
printf "and "
set_color_by_good_percentage "$success_percent"
printf "%s succeeded (%s%%)." "$succeeded" "$success_percent"
resetcolor
printf "\n"
if [ "$failed" -gt 0 ]; then
	setfgcolor $RED
	printf "Failing tests:\n"
	for test in "${failed_ids[@]}"; do
		printf "Test '%s'\n"
	done
	resetcolor
fi
cd "$oldpwd"
