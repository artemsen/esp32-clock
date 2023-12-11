#!/bin/sh -e

#
# Build firmware image
#

# check name and password for wifi connection
if [ -z "${WIFI_SSID+x}" ] || [ -z "${WIFI_PASSWORD+x}" ]; then
  echo "ERROR: WIFI name or password not defined" >&2
  echo "Use: WIFI_SSID=\"WiFiNetworkName\" WIFI_PASSWORD=\"WiFiPassword\" $0" >&2
  exit 1
fi

# prepare build directory
BUILD_DIR="build"
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# import esp-idf environment
ESPIDF_ENV="../esp-idf/export.sh"
if [ ! -f ${ESPIDF_ENV} ]; then
  git submodule update --init
fi
. ${ESPIDF_ENV}

# build the project
cmake ..
make -j"$(nproc)"
