#!/bin/sh
# SPDX-License-Identifier: LGPL-3.0-or-later
# Checks or fixes the includes of each project header. clang-tidy's misc-include-cleaner only
# analyzes translation units, so it does not cover headers on their own.
# usage: include_cleaner.sh <check|fix> <clang-include-cleaner> <build-dir> <ignore-regex> <extra-arg|""> <path>...
mode=$1; exe=$2; build=$3; ignore=$4; extra=$5; shift 5
export mode exe build ignore extra
find "$@" -name '*.hpp' -print0 | xargs -0 -P "$(getconf _NPROCESSORS_ONLN)" -n 1 sh -c '
    file=$1
    run() { "$exe" -p "$build" --ignore-headers="$ignore" ${extra:+"--extra-arg=$extra"} "$@" "$file"; }
    out=$(run --print=changes 2>/dev/null) || { echo "$file: could not be analyzed"; run --print=changes 2>&1 | grep error: ; exit 1; }
    [ -z "$out" ] && exit 0
    if [ "$mode" = fix ]; then run --edit 2>/dev/null; exit $?; fi
    printf "%s:\n%s\n" "$file" "$out"
    exit 1
' sh
