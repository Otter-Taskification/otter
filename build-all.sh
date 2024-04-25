#! /bin/env bash

if [ -z "$1" ]; then
    printf "Error: didn't specify a compiler preset (clang, gcc)\n" >&2
    exit 1
fi

if [ -z "$2" ]; then
    printf "Error: didn't specify an install prefix\n" >&2
    exit 2
fi

# Generate all presets for the given compiler
cmake --list-presets                                      \
    | sed 's/\"//g' | grep "^\s*${1}"                     \
    | tr -s " "     | cut -d " " -f 2                     \
    | xargs -L 1 cmake --log-level VERBOSE --preset


# Build all the presets above
cmake --build --list-presets                              \
    | sed 's/\"//g' | grep "^\s*${1}"                     \
    | tr -s " "     | cut -d " " -f 2                     \
    | xargs -L 1 cmake --build --preset


# Install all the presets generated above
cmake --build --list-presets                              \
    | sed 's/\"//g' | grep "^\s*${1}"                     \
    | tr -s " "     | cut -d " " -f 2                     \
    | xargs -I '{}' cmake --install build/'{}' --prefix "${2}"/'{}'
