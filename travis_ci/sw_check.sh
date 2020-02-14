# The directories of the sw source files
srcdir_examples=/mnt/data/sw/example
srcdir_bootloader=/mnt/data/sw/bootloader

# List files
ls -al $srcdir_examples
ls -al $srcdir_bootloader

# Try to compile all example projects
make -C $srcdir_examples compile
make -C $srcdir_bootloader all
