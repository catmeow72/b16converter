#!/bin/bash
oldpwd="$(pwd)"
cd "$(dirname "$0")"
meson setup builddir
meson compile -C builddir || exit $?
images=("TEST.png" "PACK.png")
outdir="testout"
mkdir "$outdir"
for img in "${images[@]}"; do
	for bpp in 2 4 8; do
		for size in "8x8" "16x16" "32x32" "64x64" "320x240" "640x480"; do
			width="$(echo -n "$size" | cut -dx -f1)"
			height="$(echo -n "$size" | cut -dx -f2)"
			name="$(echo -n "$img" | sed 's/\.png$//')"
			name="$(printf "%s.%sP.%sB" "$name" "$width" "$bpp")"
			./builddir/graphicsconverter -in "$img" -out "$outdir/$name.B16" -bpp "$bpp" -resize "$width" "$height" -border 15 0 15
			./builddir/graphicsconverter -in "$img" -out "$outdir/$name.D.B16" -bpp "$bpp" -resize "$width" "$height" -dither -border 15 0 15
			./builddir/graphicsconverter -reverse -in "$outdir/$name.B16" -out "$outdir/$name.PNG" -resize "$width" "$height"
			./builddir/graphicsconverter -reverse -in "$outdir/$name.D.B16" -out "$outdir/$name.D.PNG" -resize "$width" "$height" -dither
		done
	done
done

cd "$oldpwd"
