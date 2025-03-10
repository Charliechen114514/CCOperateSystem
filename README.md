# CCOperateSystem

![](https://img.shields.io/badge/Language-C>=C90-informational?logo=C&logoColor=#A8B9CC&color=#A8B9CC)![](https://img.shields.io/badge/Language-ASM-red)![STILL_IN_MAINTAINS](https://img.shields.io/badge/Maintains-YES-red)![License](https://img.shields.io/badge/license-GNUv3-yellow) ![Documentation](https://img.shields.io/badge/Documentation-WIP-brightgreen)![Documentation](https://img.shields.io/badge/Code-TestOnPassAsBasic-brightgreen)

 一句话：一个简单的教程尝试教你如何使用 C 语言和典型的 nasm 在现代工具链中制作操作系统。点击下面的语言标签切换本文档的语言~

- 简体中文：[![GUI](https://img.shields.io/badge/使用-简体中文-red)](README.md)

- English：[![GUI](https://img.shields.io/badge/Reading_Language-English-red)](README_EN.md)

![GUI](https://img.shields.io/badge/Introduction-What_is_CC_Operating_System-blue)

CCOperatingSystem是一种非常简单的操作系统，可以在bochs虚拟机中运行。要启动，您需要设置您的工作环境，目前，我可以保证除了我特别标记的代码之外，所有代码都可以使用最新的 gcc 和 nasm 进行编译。

![GUI](https://img.shields.io/badge/Setup-Try_The_Run-yellow)

![文档](https://img.shields.io/badge/TOOLS-GCC>=4.4.7-brightgreen)![文档](https://img.shields.io/badge/TOOLS-nasm>=2.16-red)![文档](https://img.shields.io/badge/Environment-Bochs_Only_Current-blue)

​	 所以，你需要做的事情非常简单。最重要的是，我们只需要

- `gcc（至少检测过的版本> = 4.4.7，最高为最新版本）(现在笔者测试的版本是gcc version 14.2.1 20250207 (GCC), 但是，笔者测试疑似gcc 14.2.1存在栈溢出风险(Invalid OP Code错误)，导致操作系统崩溃，WIP: 修复这个错误。完全没问题的版本是Ubuntu的自带gcc, 13.3.0是完全没有任何问题的)`
- `nasm（至少检测过的最新版本）(NASM version 2.16.03 compiled on May  4 2024)`
- `bochs（检测过的是使用 2.8，当然其他的也可以，需要自己魔改，请参考其他教程灵活调整您的配置！`

​	请注意，本操作系统目前只能在bochs上跑，qemu会卡死在硬盘读取上，本错误长期不打算修复，如果你有更好的对Loader的实现，请Issue我进一步探讨！

![GUI](https://img.shields.io/badge/Screenshots-See_The_Runtime_Screenshots-red)

​	无冗余信息的启动界面

![runtimes1](./readme.assets/runtimes-boot.png)

​	看看用法：

![runtimes-help](./readme.assets/runtimes-help.png)

​	清空（CTRL + L清屏幕，CTRL + U清输入）

![runtimes-show-clearprev](./readme.assets/runtimes-show-clearprev.png)

![runtimes-show-clearafter](./readme.assets/runtimes-show-clearafter.png)

​	ps进程显示

![ps](./readme.assets/image-20250302204310767.png)	

操作文件系统

![handling-operating-filesystem](./readme.assets/image-20250302204346151.png)

![filesystem-1](./readme.assets/filesystem-1.png)

 	异常处理显示：

![page-faults-hanlder](./readme.assets/page-faults-hanlder.png)



![GUI](https://img.shields.io/badge/Documentation-Where_And_How_Should_I_Start-yellow)

​	从这里开始！

> :link: :point_right:  [Preface of the tour!](./Documentations/README_EN.md)
>
> :link: :point_right:  [前言！](./Documentations/README.md)

- [README_EN](README_EN.md)  
- [README](README.md)  

- Documentations/
  - [README_EN](Documentations/README_EN.md)  
  - [README](Documentations/README.md)  

  - 第 2 章 启动引导（Start From MBR）  
    - [2.1 Cody First](Documentations/2_Start_From_MBR/2.1_Cody_First.md)  
    - [2.2 Next For Details](Documentations/2_Start_From_MBR/2.2_Next_For_Details.md)  
    - [2.3 Start Our Loader Road](Documentations/2_Start_From_MBR/2.3_Start_Our_Loader_Road.md)  

  - 第 3 章 实现加载器（Implement A Loader）  
    - [3.1 Start From ProtectMode](Documentations/3_Implement_A_Loader/3.1_Start_From_ProtectMode.md)  
    - [3.2 Detect Our Memory](Documentations/3_Implement_A_Loader/3.2_Detect_Our_Memory.md)  
    - [3.3 Setup Page Tables](Documentations/3_Implement_A_Loader/3.3_setup_page_tables.md)  
    - [3.4 Final Load Kernel](Documentations/3_Implement_A_Loader/3.4_final_load_kernel.md)  

  - 第 4 章 改进内核主模块（Better MainKernel）  
    - [4.1 C & ASM Program](Documentations/4_Better_MainKernel/4.1_C_ASM_Program.md)  
    - [4.2 实现简单打印库](Documentations/4_Better_MainKernel/4.2_Implement_Our_SimplePrintLibrary.md)  

  - 第 5 章 中断（Interrupt）  
    - [5.1 Interrupt](Documentations/5_Interrupt/5.1_Interrupt.md)  
    - [5.2 Programming 8259A](Documentations/5_Interrupt/5,2_Programming_8259A.md)  
    - [5.3 实现中断子系统（一）](Documentations/5_Interrupt/5.3_Implement_InterruptSubSystem_1.md)  
    - [5.4 实现中断子系统（二）](Documentations/5_Interrupt/5.4_Implement_InterruptSubSystem_2.md)  

  - 第 6 章 构建内核库（Kernel Library）  
    - [6.1 string.h](Documentations/6_Setup_Our_Kernel_Library/6.1_start_from_string_h.md)  
    - [6.2 内存池的 bitmap](Documentations/6_Setup_Our_Kernel_Library/6.2_implement_bitmaps_for_mempools.md)  
    - [6.3 实现链表](Documentations/6_Setup_Our_Kernel_Library/6.3_implement_list.md)  

  - 第 7 章 内存管理  
    - [7.1 获取内存信息](Documentations/7_Memory_Management/7.1_Fetch_Our_Memory.md)  
    - [7.2 建立分页机制](Documentations/7_Memory_Management/7.2_Setup_PageFetch.md)  

  - 第 8 章 线程管理  
    - [8.1 实现内核线程](Documentations/8_Thread_Management/8.1_Implement_KernelThread.md)  
    - [8.2 实现线程切换](Documentations/8_Thread_Management/8.2_implement_switch_thread.md)  
    - [8.3 加锁提升线程切换安全性](Documentations/8_Thread_Management/8.3_make_lock_to_switch_safer.md)  

  - 第 9 章 输入子系统  
    - [9.1 驱动键盘](Documentations/9_Boost_BasicInputSubsystem/9.1_implement_driving_keyboard.md)  
    - [9.2 完善输入子系统](Documentations/9_Boost_BasicInputSubsystem/9.2_finish_input_subsystem.md)  
    - [9.3 输入子系统改进](Documentations/9_Boost_BasicInputSubsystem/9.3_finish_input_subsystem_2.md)  

  - 第 10 章 用户线程（User Thread）  
    - [10.1 实现 TSS](Documentations/10_Implement_User_Thread/10.1_implement_tss.md)  
    - [10.2 实现用户进程](Documentations/10_Implement_User_Thread/10.2_implement_user_proc.md)  

  - 第 11 章 高级内核特性  
    - [11.1 实现系统调用](Documentations/11_advanced_kernel/11.1_implement_syscall.md)  
    - [11.2~3 实现 printf](Documentations/11_advanced_kernel/11.2_3_implement_printf.md)  
    - [11.4~6 实现 malloc/free](Documentations/11_advanced_kernel/11.4_5_6_impelement_malloc_free.md)  

  - 第 12 章 硬盘驱动  
    - [12.1 完善 IDE 驱动](Documentations/12_harddisk_driver/12.1_finish_ide_driver.md)  

  - 第 13 章 文件系统  
    - [13.1 检测文件系统](Documentations/13_filesystem/13.1_detect_filesystem.md)  
    - [13.2 挂载文件系统](Documentations/13_filesystem/13.2_mount_filesystem.md)  
    - [13.3 实现文件描述符](Documentations/13_filesystem/13.3_impl_fd.md)  
    - [13.4 文件写入](Documentations/13_filesystem/13.4_file_write.md)  
    - [13.5 文件读取](Documentations/13_filesystem/13.5_file_read.md)  
    - [13.6 lseek](Documentations/13_filesystem/13.6_lseek.md)  
    - [13.7 文件删除](Documentations/13_filesystem/13.7_file_del.md)  
    - [13.8 目录项操作](Documentations/13_filesystem/13.8_dirent_op.md)  
    - [13.9 pwd](Documentations/13_filesystem/13.9_pwd.md)  
    - [13.10 stat](Documentations/13_filesystem/13.10_stat.md)  

  - 第 14 章 用户进程实用功能  
    - [14.1 fork](Documentations/14_user_proc_utils/14.1_fork.md)  
    - [14.2 简单 shell](Documentations/14_user_proc_utils/14.2_simple_shell.md)  
    - [14.3 改进 shell](Documentations/14_user_proc_utils/14.3_better_shell.md)  
    - [14.4 更进一步的 shell](Documentations/14_user_proc_utils/14.4_better_shell2.md)  
    - [14.5 exec](Documentations/14_user_proc_utils/14.5_exec.md)  
    - [14.6 wait/exit](Documentations/14_user_proc_utils/14.6_wait_exit.md)  
    - [14.7 管道 pipe](Documentations/14_user_proc_utils/14.7_pipe.md)  

  - 第 15 章 完结  
    - [最终章 finally....](Documentations/15_THIS_IS_THE_END/finally....md)  

  - 附录 Bonus  
    - [Debug Bochs](Documentations/bonus/Debug_Bochs.md)  
    - [Inline ASM](Documentations/bonus/inline_asm.md)  
    - [保护模式概述](Documentations/bonus/ProtectMode.md)  
    - [实模式概述](Documentations/bonus/Real_Mode.md)  
    - [Intel 切换任务机制](Documentations/bonus/chap10/Intel_Switch_Task.md)  
    - [创建硬盘镜像 bximage](Documentations/bonus/chap2/use_bximage_to_make_hard_disk.md)  
    - [Bochs 调试启动代码](Documentations/bonus/chap2/using_bochs_to_setup_code.md)  
    - [ELF 格式解析](Documentations/bonus/chap3/elf.md)  
    - [Intel x86_64 调用与返回（英文）](Documentations/bonus/chap8/Intel_X86_64 Call_And_Reture_Procedure.md)  
    - [Intel x86_64 调用与返回（中文）](Documentations/bonus/chap8/Intel_X86_64_调用和返回过程.md)  

  - 附录 Setup 配置指导  
    - [ArchLinux 配置](Documentations/setups/ArchLinux.md)  
    - [Ubuntu 配置](Documentations/setups/Ubuntu.md)  
    - [WSL Arch 配置](Documentations/setups/WSL-Arch.md)  
    - [WSL Ubuntu 配置](Documentations/setups/WSL-Ubuntu.md)  
    - [README_EN](Documentations/setups/README_EN.md)  
    - [README](Documentations/setups/README.md)  

![Ref And Recom](https://img.shields.io/badge/Reference_And_Recommendations-Some_also_os_project-red)

​	如果你想让你的操作系统可以实现更好，更加连贯的抽象，想挑战自己的话，考虑一个正在WIP的，使用纯粹GNU工具链的kdemo:

- Author: [Dessera (Dessera)](https://github.com/Dessera)
- KDemo: [Dessera/kdemo](https://github.com/Dessera/kdemo)

​	如果你更加倾向于稳定的实现，并可以接受较老工具链。请考虑这位的

- Author: [Cooi-Boi (Love6)](https://github.com/Cooi-Boi)
- Tidy-OS: [Cooi-Boi/Tiny-OS: 《操作系统真象还原》自写源码实现 并于CSDN上面详细记录操作系统的整个实现流程 包括Debug步骤与书籍中错误勘误 Bochs2.6.8 Gcc4.4 本书除最后三个小功能其余全部实现 6k行左右的代码 希望能帮到各位 ^^](https://github.com/Cooi-Boi/Tiny-OS/tree/master)

​	一个更加完善的讲解实现：

- [yifengyou (游~游~游)](https://github.com/yifengyou)
- [yifengyou/os-elephant: 《操作系统真象还原》源码及学习笔记（os-elephant）还原真相](https://github.com/yifengyou/os-elephant)



![GUI](https://img.shields.io/badge/Other_Implementations-Some_also_os_project-green)

​	本教程的参考资料几乎是派生自《操作系统还原真相》此书。感兴趣的朋友可以购买一本支持郑刚老师！
