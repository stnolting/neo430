# The directories of the hw source files
srcdir_core=/mnt/data/rtl/core
srcdir_top_templates=/mnt/data/rtl/top_templates
srcdir_sim=/mnt/data/sim

# List files
ls -al $srcdir_core
ls -al $srcdir_top_templates
ls -al $srcdir_sim

# Analyse sources
ghdl -a --work=neo430 $srcdir_core/neo430_package.vhd $srcdir_core/neo430_bootloader_image.vhd $srcdir_core/neo430_application_image.vhd $srcdir_core/*.vhd $srcdir_top_templates/*.vhd $srcdir_sim/*.vhd


