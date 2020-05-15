#!/bin/bash

# Abort if any command returns != 0
set -e

# The directories of the sw source files
srcdir_examples=/mnt/data/sw/example
srcdir_bootloader=/mnt/data/sw/bootloader

# List files
ls -al $srcdir_examples
ls -al $srcdir_bootloader

# check toolchain
make -C $srcdir_examples/blink_led check

# Try to compile all example + bootloader
make -C $srcdir_examples clean_all info compile
make -C $srcdir_bootloader clean_all info all
