#! /bin/env bash

set -e

RED="\e[1;31m"
GRN="\e[1;32m"
ENDCOLOR="\e[0m"

__echo_error() {
    echo -e "${RED}######"
    echo -e "${RED}###### Error:${ENDCOLOR} ${1}"
    echo -e "${RED}######${ENDCOLOR}"
}

__echo_success() {
    echo -e "${GRN}✓✓✓✓✓✓ OK:${ENDCOLOR} ${1}"
}

__echo_info() {
    echo "====== ${1}"
}

if [ -z "$1" ]; then
    __echo_error "didn't specify a compiler preset (clang, gcc, intel)" >&2
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

__list_presets() {
    cmake --list-presets | sed 's/\"//g' | grep "^\s*${1}" | tr -s " "     | cut -d " " -f 2
}

__configure_preset() {
    __echo_info "${1}: configuring"
    __echo_info
    (set -x; cmake --log-level VERBOSE --preset "${1}")
}

__build_preset() {
    __echo_info "${1}: building"
    __echo_info
    (set -x; cmake --build --preset "${1}")
}

__install_preset() {
    __echo_info "${1}: installing"
    __echo_info
    (set -x; cmake --install build/"${1}" --prefix "${2}")
}

__link_installed_module() {
    __echo_info "${1}: link module file"
    (set -x; ln -s "${2}/etc/modulefiles/otter/otter" "${3}")
}

__echo_info "compiler preset:   ${1}"
__echo_info "install prefix:    ${2}"
__echo_info "symlink prefix:    ${3}"
__echo_info
echo ""

for preset in $(__list_presets ${1}); do
    __configure_preset "${preset}"
    if __build_preset "${preset}"; then
        install_prefix="${2}"/"${preset}"
        module_symlink="${3}"/"${preset}"
        __install_preset "${preset}" "${install_prefix}"
        if [ ! -f "${module_symlink}" ]; then
            __link_installed_module "${preset}" "${install_prefix}" "${module_symlink}"
        fi
        __echo_success "${preset} complete!"
    else
        __echo_error "${preset} preset failed to build"
    fi
done
