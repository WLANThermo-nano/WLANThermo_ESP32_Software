#!/usr/bin/env python3

import argparse
import os
import sys
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument("-f", "--firmware")
parser.add_argument("backtrace", nargs='*', default=None)
args = parser.parse_args()

def get_latest_fw(fw_type=''):
    if len(fw_type) > 0:
        latest = f'build/{fw_type}_firmware_latest.elf'
    else:
        latest = 'build/firmware_latest.elf'

    if not os.path.exists(latest):
        print("Failed to find a firmware containing the string {}".format(fw_type))
        sys.exit(0)

    try:
        latest = os.path.join('build', os.readlink(latest))
    except OSError:
        pass

    return latest

if args.firmware is None:
    firmware = get_latest_fw()
else:
    if os.path.exists(args.firmware):
        if ".elf" in args.firmware:
            firmware = args.firmware
        if "_merged.bin" in args.firmware:
            firmware = args.firmware.replace("_merged.bin", ".elf")
    else:
        firmware = get_latest_fw(args.firmware)

print(f'Using firmware: {firmware}')

os.system("pio pkg exec xtensa-esp32-elf-addr2line -- -pfiaC -e {} {}".format(firmware, " ".join(args.backtrace)))
