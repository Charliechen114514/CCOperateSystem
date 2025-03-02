#!/bin/sh

if [ ! -e disk_image ]; then
    make create_disk
fi

echo -e "label: dos
unit: sectors

disk.img1 : start=2048,  size=30208, type=83
disk.img4 : start=32256, size=131040, type=5
disk.img5 : start=34304, size=16096, type=66
disk.img6 : start=52448, size=23152, type=66
disk.img7 : start=77648, size=13072, type=66
disk.img8 : start=93744, size=27216, type=66
disk.img9 : start=123008, size=40288, type=66
" | sfdisk disk.img

echo "Now display the disk info: "
fdisk -l disk.img
