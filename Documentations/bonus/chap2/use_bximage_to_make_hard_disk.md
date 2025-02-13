# 使用BxImage制作启动盘的教程

## 前言

​	无论你是使用包管理器，还是使用自己编译的方式，我们最后的安装文件夹下都会存在一个叫做bximage的可执行文件，他是一个简单的迷你的创建软盘和硬盘的小软件。

```
========================================================================
                                bximage
  Disk Image Creation / Conversion / Resize and Commit Tool for Bochs
                                  $Id$
========================================================================

1. Create new floppy or hard disk image
2. Convert hard disk image to other format (mode)
3. Resize hard disk image
4. Commit 'undoable' redolog to base image
5. Disk image info

0. Quit

Please choose one [0] 
```

​	我们需要制作的是一个硬件盘，所以选1，回车

```
Do you want to create a floppy disk image or a hard disk image?
Please type hd or fd. [hd] 
```

​	默认是硬盘，回车

```
What kind of image should I create?
Please type flat, sparse, growing, vpc or vmware4. [flat] 
```

​	默认的，回车

```
Choose the size of hard disk sectors.
Please type 512, 1024 or 4096. [512] 
```

​	我们选择512。也就是默认的（为什么是512，想想我怎么说MBR的特征的）

```
Enter the hard disk size in megabytes, between 10 and 8257535
[10] 60 
```

​	我们创建一个60M大小的磁盘，这样的话够用到我们操作系统手搓教程的结尾。

```
What should be the name of the image?
[c.img] boot.img 
```

​	然后就好了，当然，名称自己任意！

​	当然他还有其他的功能，比如说软硬盘转化，重新设置硬盘大小等等功能，我们这里的教程都不会用到，感兴趣的朋友请自行查阅！