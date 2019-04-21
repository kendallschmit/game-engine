#!/bin/sh
DIR="$1"
OUT="$2"
shift 2
case "$DIR" in "" | ".")
    gcc-8 -MM -MG "$@" | sed -e "s@^\(.*\)\.o:@$OUT\1.d $OUT\1.o:@"
;;
*)
    gcc-8 -MM -MG "$@" | sed -e "s@^\(.*\)\.o:@$OUT$DIR\1.d $OUT$DIR\1.o:@"
;;
esac
