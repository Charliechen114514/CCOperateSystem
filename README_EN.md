# CCOperateSystem

![](https://img.shields.io/badge/Language-C>=C90-informational?logo=C&logoColor=#A8B9CC&color=#A8B9CC)![](https://img.shields.io/badge/Langua ge-ASM-red)![STILL_IN_MAINTAINS](https://img.shields.io/badge/Maintains-YES-red)![License](https://img.shields.io/badge/license-GNUv3-yellow) ![Documentation](https://img.shields.io/badge/Documentation-WIP-brightgreen)![Documentation](https://img.shields.io/badge/Code-TestOnPassAsBasic-brightgreen) 

In one sentence: A simple tutorial tries to teach you how to make an operating system in a modern toolchain using C language and typical nasm. Simply Click the label below to switch the language :)

- Simplified Chinese: [![GUI](https://img.shields.io/badge/使用-简体中文-red)](README.md)

- English: [![GUI](https://img.shields.io/badge/Reading_Language-English-red)](README_EN.md)

![GUI](https://img.shields.io/badge/Introduction-What_is_CC_Operating_System-blue)

CCOperatingSystem is a very simple operating system that can run in a bochs virtual machine. To start, you need to set up your working environment. At present, I can guarantee that all codes can be compiled with the latest gcc and nasm except for the codes I specially marked.

![GUI](https://img.shields.io/badge/Setup-Try_The_Run-yellow)

![Documentation](https://img.shields.io/badge/TOOLS-GCC>=4.4.7-brightgreen)![Documentation](https://img.shields.io/badge/TOOLS-nasm>=2.16-red)![Documentation](https://img.shields.io/badge/Environment-Bochs_Only_Current-blue)

 So, what you need to do is very simple. Most importantly, we only need

- `gcc (at least the tested version > = 4.4.7, the latest version at most) (the version I tested is gcc version 14.2.1 20250207 (GCC), but the author tested that gcc 14.2.1 has a stack overflow risk (Invalid OP Code error), causing the operating system to crash. WIP: fix this error. The version that has no problems is Ubuntu's built-in gcc, 13.3.0 has no problems at all)`
- `nasm (at least the latest version tested) (NASM version 2.16.03 compiled on May 4 2024)`
- `bochs (the tested version is 2.8, of course, other versions are also OK, you need to modify it yourself, please refer to other tutorials to flexibly adjust your configuration!`

Please note that this operating system can only run on bochs at present, and qemu will get stuck on hard disk reading. This error is not planned to be fixed for a long time. If you have a better implementation of Loader, please issue me for further discussion!

![GUI](https://img.shields.io/badge/Screenshots-See_The_Runtime_Screenshots-red)

 Startup interface without redundant information

![runtimes1](./readme.assets/runtimes-boot.png)

 See the usage:

![runtimes-help](./readme.assets/runtimes-help.png)

 Clear (CTRL + L to clear the screen, CTRL + U clear input)

![runtimes-show-clearprev](./readme.assets/runtimes-show-clearprev.png)

![runtimes-show-clearafter](./readme.assets/runtimes-show-clearafter.png)

 ps process display

![ps](./readme.assets/image-20250302204310767.png)

Operating file system

![handling-operating-filesystem](./readme.assets/image-20250302204346151.png)

![filesystem-1](./readme.assets/filesystem-1.png)

Exception handling display:

![page-faults-hanlder](./readme.assets/page-faults-hanlder.png)

![GUI](https://img.shields.io/badge/Documentation-Where_And_How_Should_I_Start-yellow)

 Start here!

> :link: :point_right: [Preface of the tour!](./Documentations/README_EN.md)
>
> :link: :point_right: [Preface! ](./Documentations/README.md)

- [README_EN](README_EN.md)  
- [README](README.md)  

- Documentations/
  - [README_EN](Documentations/README_EN.md)  
  - [README](Documentations/README.md)  

  - Chapter 2 Boot from MBR  
    - [2.1 Cody First](Documentations/2_Start_From_MBR/2.1_Cody_First.md)  
    - [2.2 Next For Details](Documentations/2_Start_From_MBR/2.2_Next_For_Details.md)  
    - [2.3 Start Our Loader Road](Documentations/2_Start_From_MBR/2.3_Start_Our_Loader_Road.md)  

  - Chapter 3 Implement a Loader  
    - [3.1 Start From ProtectMode](Documentations/3_Implement_A_Loader/3.1_Start_From_ProtectMode.md)  
    - [3.2 Detect Our Memory](Documentations/3_Implement_A_Loader/3.2_Detect_Our_Memory.md)  
    - [3.3 Setup Page Tables](Documentations/3_Implement_A_Loader/3.3_setup_page_tables.md)  
    - [3.4 Final Load Kernel](Documentations/3_Implement_A_Loader/3.4_final_load_kernel.md)  

  - Chapter 4 Improve Main Kernel  
    - [4.1 C & ASM Program](Documentations/4_Better_MainKernel/4.1_C_ASM_Program.md)  
    - [4.2 Implement Simple Print Library](Documentations/4_Better_MainKernel/4.2_Implement_Our_SimplePrintLibrary.md)  

  - Chapter 5 Interrupts  
    - [5.1 Interrupt](Documentations/5_Interrupt/5.1_Interrupt.md)  
    - [5.2 Programming 8259A](Documentations/5_Interrupt/5,2_Programming_8259A.md)  
    - [5.3 Implement Interrupt Subsystem (Part 1)](Documentations/5_Interrupt/5.3_Implement_InterruptSubSystem_1.md)  
    - [5.4 Implement Interrupt Subsystem (Part 2)](Documentations/5_Interrupt/5.4_Implement_InterruptSubSystem_2.md)  

  - Chapter 6 Kernel Library  
    - [6.1 string.h](Documentations/6_Setup_Our_Kernel_Library/6.1_start_from_string_h.md)  
    - [6.2 Bitmaps for Memory Pools](Documentations/6_Setup_Our_Kernel_Library/6.2_implement_bitmaps_for_mempools.md)  
    - [6.3 Implement List](Documentations/6_Setup_Our_Kernel_Library/6.3_implement_list.md)  

  - Chapter 7 Memory Management  
    - [7.1 Fetch Memory Info](Documentations/7_Memory_Management/7.1_Fetch_Our_Memory.md)  
    - [7.2 Setup Paging](Documentations/7_Memory_Management/7.2_Setup_PageFetch.md)  

  - Chapter 8 Thread Management  
    - [8.1 Implement Kernel Thread](Documentations/8_Thread_Management/8.1_Implement_KernelThread.md)  
    - [8.2 Implement Thread Switching](Documentations/8_Thread_Management/8.2_implement_switch_thread.md)  
    - [8.3 Locking for Safer Switch](Documentations/8_Thread_Management/8.3_make_lock_to_switch_safer.md)  

  - Chapter 9 Input Subsystem  
    - [9.1 Implement Keyboard Driver](Documentations/9_Boost_BasicInputSubsystem/9.1_implement_driving_keyboard.md)  
    - [9.2 Finish Input Subsystem](Documentations/9_Boost_BasicInputSubsystem/9.2_finish_input_subsystem.md)  
    - [9.3 Improve Input Subsystem](Documentations/9_Boost_BasicInputSubsystem/9.3_finish_input_subsystem_2.md)  

  - Chapter 10 User Threads  
    - [10.1 Implement TSS](Documentations/10_Implement_User_Thread/10.1_implement_tss.md)  
    - [10.2 Implement User Process](Documentations/10_Implement_User_Thread/10.2_implement_user_proc.md)  

  - Chapter 11 Advanced Kernel Features  
    - [11.1 Implement Syscall](Documentations/11_advanced_kernel/11.1_implement_syscall.md)  
    - [11.2~3 Implement printf](Documentations/11_advanced_kernel/11.2_3_implement_printf.md)  
    - [11.4~6 Implement malloc/free](Documentations/11_advanced_kernel/11.4_5_6_impelement_malloc_free.md)  

  - Chapter 12 Hard Disk Driver  
    - [12.1 Finish IDE Driver](Documentations/12_harddisk_driver/12.1_finish_ide_driver.md)  

  - Chapter 13 File System  
    - [13.1 Detect File System](Documentations/13_filesystem/13.1_detect_filesystem.md)  
    - [13.2 Mount File System](Documentations/13_filesystem/13.2_mount_filesystem.md)  
    - [13.3 Implement File Descriptor](Documentations/13_filesystem/13.3_impl_fd.md)  
    - [13.4 File Write](Documentations/13_filesystem/13.4_file_write.md)  
    - [13.5 File Read](Documentations/13_filesystem/13.5_file_read.md)  
    - [13.6 lseek](Documentations/13_filesystem/13.6_lseek.md)  
    - [13.7 File Deletion](Documentations/13_filesystem/13.7_file_del.md)  
    - [13.8 Directory Entry Operations](Documentations/13_filesystem/13.8_dirent_op.md)  
    - [13.9 pwd](Documentations/13_filesystem/13.9_pwd.md)  
    - [13.10 stat](Documentations/13_filesystem/13.10_stat.md)  

  - Chapter 14 User Process Utilities  
    - [14.1 fork](Documentations/14_user_proc_utils/14.1_fork.md)  
    - [14.2 Simple Shell](Documentations/14_user_proc_utils/14.2_simple_shell.md)  
    - [14.3 Better Shell](Documentations/14_user_proc_utils/14.3_better_shell.md)  
    - [14.4 Enhanced Shell](Documentations/14_user_proc_utils/14.4_better_shell2.md)  
    - [14.5 exec](Documentations/14_user_proc_utils/14.5_exec.md)  
    - [14.6 wait/exit](Documentations/14_user_proc_utils/14.6_wait_exit.md)  
    - [14.7 pipe](Documentations/14_user_proc_utils/14.7_pipe.md)  

  - Chapter 15 The End  
    - [Final Chapter](Documentations/15_THIS_IS_THE_END/finally....md)  

  - Appendix Bonus  
    - [Debugging Bochs](Documentations/bonus/Debug_Bochs.md)  
    - [Inline ASM](Documentations/bonus/inline_asm.md)  
    - [Overview of Protected Mode](Documentations/bonus/ProtectMode.md)  
    - [Overview of Real Mode](Documentations/bonus/Real_Mode.md)  
    - [Intel Task Switching](Documentations/bonus/chap10/Intel_Switch_Task.md)  
    - [Use bximage to Make Hard Disk](Documentations/bonus/chap2/use_bximage_to_make_hard_disk.md)  
    - [Use Bochs to Debug Boot Code](Documentations/bonus/chap2/using_bochs_to_setup_code.md)  
    - [ELF Format](Documentations/bonus/chap3/elf.md)  
    - [Intel x86_64 Call & Return (EN)](Documentations/bonus/chap8/Intel_X86_64 Call_And_Reture_Procedure.md)  
    - [Intel x86_64 Call & Return (CN)](Documentations/bonus/chap8/Intel_X86_64_调用和返回过程.md)  

  - Appendix Setup Guides  
    - [ArchLinux Setup](Documentations/setups/ArchLinux.md)  
    - [Ubuntu Setup](Documentations/setups/Ubuntu.md)  
    - [WSL-Arch Setup](Documentations/setups/WSL-Arch.md)  
    - [WSL-Ubuntu Setup](Documentations/setups/WSL-Ubuntu.md)  
    - [README_EN](Documentations/setups/README_EN.md)  
    - [README](Documentations/setups/README.md)  

![Ref And Recom](https://img.shields.io/badge/Reference_And_Recommendations-Some_also_os_project-red)

 If you want to make your operating system achieve better and more coherent abstractions, and want to challenge yourself, consider a WIP kdemo that uses a pure GNU toolchain:

- Author: [Dessera (Dessera)](https://github.com/Dessera)
- KDemo: [Dessera/kdemo](https://github.com/Dessera/kdemo)

 If you prefer a stable implementation and can accept an older toolchain. Please consider this

- Author: [Cooi-Boi (Love6)](https://github.com/Cooi-Boi)
- Tidy-OS: [Cooi-Boi/Tiny-OS: "Restoring the True Image of the Operating System" self-written source code implementation and detailed record of the entire implementation process of the operating system on CSDN, including Debug steps and errors in the book Bochs2.6.8 Gcc4.4 This book implements all except the last three small functions. About 6k lines of code. I hope it can help you ^^](https://github.com/Cooi-Boi/Tiny-OS/tree/master)

 A more complete explanation implementation:

- [yifengyou (游~游~游)](https://github.com/yifengyou)
- [yifengyou/os-elephant: "Operating System Truth Restoration" Source Code and Study Notes (os-elephant) Restoring the Truth](https://github.com/yifengyou/os-elephant)

![GUI](https://img.shields.io/badge/Other_Implementations-Some_also_os_project-green)

 The reference materials of this tutorial are almost derived from the book "Operating System Truth Restoration". Interested friends can buy a copy to support Mr. Zheng Gang!
