#! /usr/bin/env bash

set -e

# Enforce that variables exist
: "${OTF2_VERSION_TARGET:?expected OTF2_VERSION_TARGET to be set}"
: "${OTF2_INSTALL_PREFIX:?expected OTF2_INSTALL_PREFIX to be set}"
OTF2_URL_FILE=".github/ci/static/otf2-${OTF2_VERSION_TARGET}-url"
OTF2_DIR="otf2-${OTF2_VERSION_TARGET}"
OTF2_TAR="${OTF2_DIR}.tar.gz"

if [[ ! -e "$OTF2_URL_FILE" ]]; then
    printf "OTF2_URL_FILE not found: ${OTF2_URL_FILE}\n"
    exit 1
fi
OTF2_URL="$(<"$OTF2_URL_FILE")"

printf "Downloading OTF2 ${OTF2_VERSION_TARGET} from ${OTF2_URL}\n"
wget "$(<"$OTF2_URL_FILE")" &>/dev/null
tar -xzvf "$OTF2_TAR"
rm "$OTF2_TAR"

printf "Building OTF2 ${OTF2_VERSION_TARGET}\n"
cd "$OTF2_DIR"
# Build without python bindings in CI, we never need them
PYTHON=: ./configure --prefix="${OTF2_INSTALL_PREFIX}" --enable-static --disable-shared
make

printf "Installing OTF2 ${OTF2_VERSION_TARGET} to ${OTF2_INSTALL_PREFIX}\n"
sudo make install
