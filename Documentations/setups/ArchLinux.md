# Setup In Arch Linux

​	在Arch Linux上更新环境，你需要做的是

1. 请刷新并同步你的数据库！

   ```
   yay -Syyu
   ```

2. 下载gcc, nasm和bochs

   ```
   yay -S gcc nasm bochs
   ```

3. 一个良好的习惯就是确认他们都在！

   ```
   >> gcc -v
   >> nasm -v
   >> bochs --help
   ```

4. 克隆此项目，然后，cd到任何一个随意的章节代码

   ```
   git clone https://github.com/Charliechen114514/CCOperateSystem
   ```

5. 使用make来构建代码，然后输入make upload来启动项目。如果你遇到了任何问题，可以先尝试阅读Makefile文件！

```
>> make
# Ensure that no error has been occurred
>> make upload
# This will run the codes in bochs
```

6. 现在，代码运行起来了，但是你会发现，你的终端被bochs接管之后，似乎停下来了，这是因为笔者默认给您使用的是调试模式，你只需要输入'c'并且回车，就会发现代码跑起来了！

   下面的截图是笔者运行Chap2章节的代码，你可以试试！

![](./setup/result_demo.png)



## 潜在的问题

1. 找不到VGAImage, ROMImage或者是Keyboard等文件

   请自行寻找自己编译或者是包管理下载的bochs文件将您的VGAImage文件放在何处，在bochsrc中修改指向这个文件，推介使用绝对路径

2. 待补充