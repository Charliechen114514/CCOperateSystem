# 配置我们的Boshsrc

​	配置boshsrc是一个比较枯燥的事情，我们先来看一个笔者自己查阅资料（实际上是CV的《操作系统还原真相》此书的配置做了适当的修改）

```
# My Configure in Bochs

# RAM size available
megs:               32


# BIOS and VGA BIOS
# modify the romimage and vgaromimage to the place in your operating system
romimage:           file=/usr/local/share/bochs/BIOS-bochs-latest
vgaromimage:        file=/usr/local/share/bochs/VGABIOS-lgpl-latest


# USE WHAT
boot:               disk
log:                bochs.log.out


# Configure More IO Device
mouse:              enabled=0
# set the keyboard map to the place
keyboard:           keymap=/usr/local/share/bochs/keymaps/x11-pc-us.map

# disk related
ata0:               enabled=1, ioaddr1=0x1f0, ioaddr2=0x3f0, irq=14

# Modify the boot sections, path should be the image for the boot!
ata0-master:        type=disk, path="boot.img", mode=flat
```

​	当然，我们的bochs不支持使用gdb进行调试，我们直接使用的是内置的调试器进行的调试。