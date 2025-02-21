# 说说我们的ELF文件

## 先聊一聊有的没的

​	现在，我们来说一下我们的ELF文件吧！这个会在我们需要拷贝我们使用GCC和NASM编译的主代码二进制文件的时候用到，也就是我们的Loader.S文件当中的kernel_init函数

​	毫无疑问，很多人对我们的可执行文件没有太多的了解，笔者想要说的是——他跟我们的PE格式文件可以近似认为是同一层级的概念。他们都描述了各自操作系统下的可执行文件需要长什么样子。

- PE（Portable Executable）意为可移植的可执行的文件，常见的EXE、DLL、OCX、SYS、COM都是PE格式文件，微软Windows操作系统上的程序文件（可能是间接被执行，如DLL）

- ELF（Executable and Linkable Format）意为可执行与可链接格式，一种用于二进制文件、可执行文件、目标代码、共享库和核心转储格式文件，是UNIX系统实验室（USL）作为应用程序二进制接口（Application Binary Interface，ABI）而开发和发布的，也是Linux的主要可执行文件格式。1999年，被86open项目选为x86架构上的类Unix操作系统的二进制文件标准格式，用来取代COFF。因其可扩展性与灵活性，也可应用在其它处理器、计算机系统架构的操作系统上。

​	我们在3-4节，即将编译出的kernel.bin主操作系统文件，就是一个典型的ELF32位的可执行文件。

```
charliechen@Charliechen:~/$ file kernel.bin 
kernel.bin: ELF 32-bit LSB executable, Intel 80386, version 1 (SYSV), statically linked, not stripped
```

​	查看其内部的符号也很简单。

```
charliechen@Charliechen:~/$ nm kernel.bin 
c0003ff4 d _GLOBAL_OFFSET_TABLE_
c0004000 D __bss_start
c0001510 T __x86.get_pc_thunk.ax
c0004000 D _edata
c0004000 D _end
c0001500 T main

charliechen@Charliechen:~/$ nm main.o
         U _GLOBAL_OFFSET_TABLE_
00000000 T __x86.get_pc_thunk.ax
00000000 T main
```

​	当然，这里面有我们GCC的段在，你可以使用ldscripts去控制生成一个更加干净的二进制文件。当然，笔者在Makefile里，还写了链接我们需要注意的事情

```
	ld -m elf_i386 -z noexecstack \
		${OBJ_DIR}/main.o -Ttext 0xc0001500 -e main \
		-o ${OBJ_DIR}/${KERNEL}.bin
```

​	首先，这里就对应上了为什么是Intel80386了，我们指定的！其次，我们还指定了我们的入口就是0xc0001500，函数就是main函数，因为不加指定，我们的链接器将会寻找默认的符号_start，从而给你报错。我们在写操作系统，自然需要指定我们的入口函数了。这里函数可以是任何main文件中的函数名称。

## 跟我们的kernel_init的ELF详细说明

​	所以，我们如何得到一个ELF文件呢？注意我说的，不是可执行文件，而实ELF文件，一个ELF文件指代的是一个满足ELF文件格式规范的，可以被定制成动态库，可重定位文件或者是可执行文件的文件格式规范。

| ELF 目标文件类型                  | 描述                                                         |
| --------------------------------- | ------------------------------------------------------------ |
| 待重定位文件 (relocatable file)   | 待重定位文件就是常说的目标文件，属于源文件编译后但未完成链接的半成品，它被用于与其他目标文件合并链接，以构建出二进制可执行文件或动态链接库。为什么称其为“待重定位”文件呢？原因是在该目标文件中，如果引用了其他外部文件（其他目标文件或库文件）中定义的符号（变量或者函数统称为符号），在编译阶段只能先标识出一个符号名，该符号具体的地址还不能确定，因为不知道该符号是在哪个外部文件中，而该外部文件需要被重定位后才能确定文件内的符号地址，这些重定位的工作是需要在连接的过程中完成的 |
| 共享目标文件 (shared object file) | 这就是我们常说的动态链接库。在可执行文件被加载的过程中被动态链接，成为程序代码的一部分 |
| 可执行文件 (executable file)      | 经过编译链接后的、可以直接运行的程序文件                     |

​	ELF里，就约束了我们的文件的段和节要怎么排，比如说.text，比如说.data, rodata等等玩意。当然，讨论它本身包含啥，见下表：

| 字段        | 描述                                                         |
| ----------- | ------------------------------------------------------------ |
| e_ident     | 包含5个参数，分别是class、data、version、os/ABI、ABI version |
| e_type      | Elf文件类型                                                  |
| e_machine   | Elf文件的CPU平台                                             |
| e_version   | Elf版本号                                                    |
| e_entry     | 程序的入口                                                   |
| e_phoff     |                                                              |
| e_shoff     | 段表在文件中的偏移位置                                       |
| e_word      | Elf标志位                                                    |
| e_ehsize    | Elf文件头本身大小，值为64byte表示test.o的文件头有64字节      |
| e_phentsize |                                                              |
| e_phnum     |                                                              |
| e_shentsize | 段表描述符的大小，即为                                       |
| e_shnum     | 段表描述符的数量，值为12表示test目标文件中有12个段           |
| shstrndx    | 段表字符串表所在段表数组的下标，值为9表示段表字符串表位于第9个段 |

​	这里就不再多说更多跟我们OS细节关系不大的事情了。到此为止

## Reference

[ELF文件格式学习总结 - HelloWen - 博客园](https://www.cnblogs.com/sayhellowen/p/802b5b0ad648e1a343dcd0f85513065f.html)
