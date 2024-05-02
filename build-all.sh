#! /bin/env bash

set -e

if [ -z "$1" ]; then
    printf "Error: didn't specify a compiler preset (clang, gcc, intel)\n" >&2
    exit 1
fi

if [ -z "$2" ]; then
    printf "Error: didn't specify an install prefix\n" >&2
    exit 2
fi

if [ -z "$3" ]; then
    printf "Error: didn't specify a modulefile symlink prefix\n" >&2
    exit 3
fi

for preset in $(cmake --list-presets | sed 's/\"//g' | grep "^\s*${1}" | tr -s " "     | cut -d " " -f 2); do
    (set -x; cmake --log-level VERBOSE --preset "${preset}")
    (set -x; cmake --build --preset "${preset}")
    (set -x; cmake --install build/"${preset}" --prefix "${2}"/"${preset}")
    if [ ! -f "${3}"/"${preset}" ]; then
        (set -x; ln -s "${2}"/"${preset}"/etc/modulefiles/otter/otter "${3}"/"${preset}")
    fi
done
