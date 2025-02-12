# CCOperateSystem

![](https://img.shields.io/badge/Language-C>=C90-informational?logo=C&logoColor=#A8B9CC&color=#A8B9CC)![](https://img.shields.io/badge/Language-ASM-red)![STILL_IN_MAINTAINS](https://img.shields.io/badge/Maintains-YES-red)![License](https://img.shields.io/badge/license-GNUv3-yellow) ![Documentation](https://img.shields.io/badge/Documentation-ON_THE_WAY-brightgreen)

 一句话：一个简单的教程尝试教你如何使用 C 语言和典型的 nasm 在现代工具链中制作操作系统。

- 简体中文：[![GUI](https://img.shields.io/badge/使用-简体中文-red)](README.md)

- English：[![GUI](https://img.shields.io/badge/Reading_Language-English-red)](README_EN.md)

![GUI](https://img.shields.io/badge/Introduction-What_is_CC_Operating_System-blue)

 CCOperatingSystem是一种非常简单的操作系统，可以在bochs虚拟机中运行。要启动，您需要设置您的工作环境，目前，我可以保证除了我特别标记的代码之外，所有代码都可以使用最新的 gcc 和 nasm 进行编译。

![GUI](https://img.shields.io/badge/Setup-Try_The_Run-yellow)

![文档](https://img.shields.io/badge/TOOLS-GCC>=4.4.7-brightgreen)![文档](https://img.shields.io/badge/TOOLS-nasm>=2.16-red)![文档](https://img.shields.io/badge/Environment-Bochs_Only_Current-blue)

 所以，你需要做的事情非常简单。最重要的是，我们只需要

- `gcc（至少检测过的版本> = 4.4.7，最高为最新版本）`
- `nasm（至少检测过的最新版本）`
- `bochs（检测过的是使用 2.8，当然其他的也可以，需要自己魔改，请参阅下面的documentations指引！`

 就是这样！作者已经在 WSL、Ubuntu 和 Arch Linux 中进行了测试，因此我可以为您提供设置说明！有关详细信息，请参阅以下教程！

![GUI](https://img.shields.io/badge/Screenshots-See_The_Runtime_Screenshots-red)

​	无冗余信息的启动界面

![runtimes1](./readme.assets/runtimes-boot.png)

​	看看用法：

![runtimes-help](./readme.assets/runtimes-help.png)

​	清空（CTRL + L清屏幕，CTRL + U清输入）

![runtimes-show-clearprev](./readme.assets/runtimes-show-clearprev.png)

![runtimes-show-clearafter](./readme.assets/runtimes-show-clearafter.png)

​	文件系统初试

![filesystem-1](./readme.assets/filesystem-1.png)

 	异常处理显示：

![page-faults-hanlder](./readme.assets/page-faults-hanlder.png)



![GUI](https://img.shields.io/badge/Documentation-Where_And_How_Should_I_Start-yellow)

从这里开始！

> :link: :point_right:  [Preface of the tour!](./Documentations/README_EN.md)
>
> :link: :point_right:  [前言！](./Documentations/README.md)
