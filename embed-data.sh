#!/bin/sh

# This code is released into the public domain,
# WITHOUT WARRANTY OF ANY KIND.
#
# This script can be used on GNU systems to generate object files that embed
# the contents of the specified input files.
# That data can then be accessed from C code by declaring the appropriate symbols.
#
# The advantages of this over something like bin2c are that it doesn't require
# generating a giant C file and then pumping it through a C compiler (embedding
# the data directly into an object file is much simpler). It also allows the
# alignment of the data to be easily controlled.
#
# The advantages over using objcopy or ld -b binary to do this directly is that
# it allows control of alignment, and symbol name, and allows the data size to
# be embedded. (note: `ld -r -b binary -o output input` also embeds the size,
# but does so by including a symbol whose *address* is the *size* of the data.
# This possibly allows inlining[??] but can break if things are relocated)
#
# In this script, given an input path "foo/fallback_font.ttf", three symbols
# are defined, which can be declared in C++ as:
#
#   extern "C" const char DATA_FOO_FALLBACK_FONT_TTF[];
#   extern "C" const char DATA_FOO_FALLBACK_FONT_TTF_END[];
#   extern "C" const uint64_t DATA_FOO_FALLBACK_FONT_TTF_SIZE;
#
# the X and X_END symbols refer to the beginning and end of the data.
# (ie, X_SIZE == X_END - X)

ALIGN=16

while getopts o:a: flag; do
	case "$flag" in
		o)
			OUTPATH_ARG="-o$OPTARG";
			;;
		a)
			ALIGN="$OPTARG";
			;;
		?)
			printf 'Unknown argument';
			exit 1;
			;;
	esac
done

shift $((OPTIND - 1));

{
	printf '.section .rodata\n\n'

	while test "$#" -gt 0; do
		INPATH="$1"
		SYM="$(printf 'DATA_%s' "$INPATH" | tr '/.[:lower:]' '__[:upper:]')"
		shift 1;

cat <<END_SECTION
	.global $SYM
	.global ${SYM}_END
	.global ${SYM}_SIZE
	.balign $ALIGN
$SYM:
	.incbin "$INPATH"
	.balign 1
${SYM}_END:
	.int 0
	.balign $ALIGN
${SYM}_SIZE:
	.int ${SYM}_END - ${SYM}
END_SECTION
	done
} | as $OUTPATH_ARG
