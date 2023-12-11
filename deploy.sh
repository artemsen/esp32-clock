#!/bin/sh -e

#
# Write firmware image to device flash
#

BUILD_DIR="build"

. ./esp-idf/export.sh
esptool.py \
  --chip esp32 \
  -p /dev/ttyUSB0 -b 460800 \
  --before default_reset --after hard_reset \
  write_flash --flash_mode dio --flash_size detect --flash_freq 40m \
  0x1000 "${BUILD_DIR}/bootloader/bootloader.bin" \
  0x8000 "${BUILD_DIR}/partition_table/partition-table.bin" \
  0x10000 "${BUILD_DIR}/esp32-clock.bin"
