#!/bin/bash

bochs_path=$(which bochs)

if [ -z "$bochs_path" ]; then
  echo "Bochs is not installed! install the bochs first"
  exit 1
fi

bochs_share_path=$(dirname $(dirname "$bochs_path"))/share/bochs

if [ ! -d "$bochs_share_path" ]; then
  echo "Bochs install seems not exsited! see $bochs_share_path for check"
  exit 1
fi

read -p "submit the disk_path: " disk_path

cat <<EOF > bochsrc
# My Configure in Bochs

# RAM size available
megs:               32

# BIOS and VGA BIOS
romimage:           file=$bochs_share_path/BIOS-bochs-latest
vgaromimage:        file=$bochs_share_path/VGABIOS-lgpl-latest

# USE WHAT
boot:               disk
log:                bochs.log.out

# Configure More IO Device
mouse:              enabled=0
keyboard:           keymap=$bochs_share_path/keymaps/x11-pc-us.map

# disk related
ata0:               enabled=1, ioaddr1=0x1f0, ioaddr2=0x3f0, irq=14
ata0-master:        type=disk, path="$disk_path", mode=flat

# gdb support
# gdbstub:            enabled=0, port=1234, text_base=0, data_base=0, bss_base=0
EOF

echo "Summon finished into bochsrc"
