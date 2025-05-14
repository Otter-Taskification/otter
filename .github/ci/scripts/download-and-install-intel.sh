#! /usr/bin/env bash

set -e

# Enforce that variables exist
: "${INTEL_VERSION_TARGET:?expected INTEL_VERSION_TARGET to be set}"
: "${INTEL_INSTALL_PREFIX:?expected INTEL_INSTALL_PREFIX to be set}"
INTEL_URL_FILE=".github/ci/static/intel-${INTEL_VERSION_TARGET}-url"

if [[ ! -e "$INTEL_URL_FILE" ]]; then
    printf "INTEL_URL_FILE not found: ${INTEL_URL_FILE}\n"
    exit 1
fi
INTEL_URL="$(<"$INTEL_URL_FILE")"

printf "Downloading intel installer ${INTEL_VERSION_TARGET} from ${INTEL_URL}\n"
wget --no-verbose "${INTEL_URL}" -O install.sh

printf "Installing intel ${INTEL_VERSION_TARGET}\n"
sh install.sh -a \
    --action install \
    --components intel.oneapi.lin.dpcpp-cpp-compiler \
    --install-dir "${INTEL_INSTALL_PREFIX}" \
    --silent --cli --eula accept
