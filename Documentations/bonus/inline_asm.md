# 聊一聊内联汇编

​	很快，我们就要到更加深入的部分了，我们需要理解一下内联汇编的知识。

## 所以说，我们的汇编格式，其实有两种

​	其实我们的汇编格式主要就是两种：ATT格式和Intel格式

#### 通用区别

| 区别           | Intel                                                        | AT&T                                                         | 说明                                                         |
| -------------- | ------------------------------------------------------------ | ------------------------------------------------------------ | ------------------------------------------------------------ |
| 寄存器         | 寄存器前无前缀                                               | 寄存器前有前缀%                                              |                                                              |
| 操作数顺序     | 目的操作数在左，源操作数在右                                 | 源操作数在左，目的操作数在右                                 | Intel 的设计思想是目的操作数=源操作数，所以目的操作数为左值 AT&T 的设计思想是将源操作数>目的操作数，所以目的操作数在右边 |
| 操作数指定大小 | 有关内存的操作数前要加数据类型修饰符：byte 表示 8 位，word 表示 16 位，dword 表示 32 位，如 `mov byte[0x1234],eax` | 指令的最后一个字母表示操作数大小，b 表示 8 位，w 表示 16 位，1 表示 32 位。如 `mov! %eax,var` | 在 AT&T 语法中，内存地址是第一位的，所以默认数字就是内存地址。操作数是数字就等同于操作数是内存，所以左边的 var 并没有像 Intel 语法那样加上中括号[]。如果是立即数则要加$前缀，这才表示普通的数字 |
| 立即数         | 无前缀，如 `6`                                               | 有前缀$，如 `$6`                                             |                                                              |
| 远跳转         | `jmp far segment:offset`                                     | `ljmp $segment:$offset`                                      |                                                              |
| 远调用         | `call far segment:offset`                                    | `leall $segment:$offset`                                     |                                                              |
| 远返回         | `ret far n`                                                  | `Iret $n`                                                    |                                                              |

#### 立即数表达

- **Intel 格式**：
  - 立即数没有前缀，直接使用数字。
  - 例如：`mov eax, 6` 表示将立即数 6 移动到寄存器 `eax` 中。
- **AT&T 格式**：
  - 立即数需要加上 `$` 前缀。
  - 例如：`movl $6, %eax` 表示将立即数 6 移动到寄存器 `eax` 中。

#### 寻址方式

- **Intel 格式**：
  - 内存地址用方括号 `[]` 表示。
  - 例如：`mov eax, [ebx]` 表示将 `ebx` 寄存器中的值作为内存地址，将该地址中的内容移动到 `eax` 寄存器中。
  - 复杂寻址方式：`mov eax, [ebx + ecx*4 + 8]` 表示将 `ebx + ecx*4 + 8` 计算出的地址中的内容移动到 `eax` 寄存器中。
- **AT&T 格式**：
  - 内存地址用圆括号 `()` 表示。
  - 例如：`movl (%ebx), %eax` 表示将 `ebx` 寄存器中的值作为内存地址，将该地址中的内容移动到 `eax` 寄存器中。
  - 复杂寻址方式：`movl 8(%ebx, %ecx, 4), %eax` 表示将 `ebx + ecx*4 + 8` 计算出的地址中的内容移动到 `eax` 寄存器中。

介绍这个，更多的是表达的是之后排查问题使用GCC反汇编的时候，默认使用的是ATT风格的。所以，你也可以明显的指定使用intel，笔者给出：

```
	gcc -S -masm=intel demo.c -o demo_intel.s
	gcc -S -masm=att demo.c -o demo_att.s
```

## 基本的内联汇编格式

```
asm [volatile] ("asm codes")
```

​	注意，你可能看到`__asm__`和`__volatile__`这样的关键字，他们本质上都是一样的，在GCC中被define成了相同的含义。volatile关键字是用来说明不要做优化的。

​	里面，我们要写的汇编代码就在""里面了。

#### 一些书写规则

1. 必须使用双引号括起来我们的指令，不管是如何简单的指令。
2. 一对双引号不允许跨行，如果不得不跨行，必须使用反斜杠表达转义延续
3. 指令之间必须使用分号隔开

#### 小试身手

​	我们来试试看

```c
const char* msg = "Sup! Inline ASM\n";
int n = 0;

int main()
{
    while (msg[n]){
        n++;
    }
    asm("pusha;\
        movl $4, %eax;\
        movl $1, %ebx;\
        movl msg, %ecx;\
        movl n, %edx; \
        int $0x80;\
        popa");
}
```

​	这就是一个简单的程序，注意到，跑这个例子，方便起见，需要我们安装gcc-multilib包，当然，你可以指定_start跳转进入函数完成这个工作。笔者这里偷个懒。

```makefile
EXE_FILE = demo

# -no-pie promissed the available write for n
$(EXE_FILE):
	gcc -no-pie -m32 demo.c -o $(EXE_FILE)

.PHONY: clean
clean:
	rm -f $(EXE_FILE)
```

```
$ ./demo 
Sup! Inline ASM
```

## 扩展的内联汇编语法

​	下面才是一个重点！让我们的C代码和汇编代码联动起来，扩展的汇编语法要求我们按照一个模板进行参数的选择，从而进行汇编代码的输入输出。格式如下：

```
asm [volatile] ("asm codes" : output : input : clobber/modify)
```

​	output就是将汇编的运算结果导出来到我们给定的变量上，input就是我们的数据如何给这个汇编使用。clobber/modify则是告知哪些寄存器的值需要提前缓存。

- output的格式是："操作数修饰符约束名称"(C的变量名)
- input的格式是："[操作数修饰符]约束名称"(C的变量名)

​	这里的约束，就是将我们的C操作数映射到汇编的环境当中：

#### 寄存器约束

| 约束符 | 描述                                                         |
| ------ | ------------------------------------------------------------ |
| a      | 表示寄存器 `eax/ax/al`                                       |
| b      | 表示寄存器 `ebx/bx/bl`                                       |
| c      | 表示寄存器 `ecx/cx/cl`                                       |
| d      | 表示寄存器 `edx/dx/dl`                                       |
| D      | 表示寄存器 `edi/di`                                          |
| S      | 表示寄存器 `esi/si`                                          |
| q      | 表示任意这 4 个通用寄存器之一：`eax/ebx/ecx/edx`             |
| r      | 表示任意这 6 个通用寄存器之一：`eax/ebx/ecx/edx/esi/edi`     |
| g      | 表示可以存放到任意地点（寄存器和内存）。相当于除了同 `q` 一样外，还可以让 GCC 安排在内存中 |
| A      | 把 `eax` 和 `edx` 组合成 64 位整数                           |
| f      | 表示浮点寄存器                                               |
| t      | 表示第 1 个浮点寄存器                                        |
| u      | 表示第 2 个浮点寄存器                                        |

​	你看这是一个例子

```c
#include <stdio.h>
int main(){
    int a = 1, b = 2;
    int result = 0;
    asm("addl %%ebx, %%eax": "=a"(result) : "a"(a), "b"(b));
    printf("result: %d\n", result); 
}
```

​	我们把a和b分别送到我们的寄存器当中去，执行的结果放回来到我们的result当中去（结果在eax，所以是=a）

​	GCC当然还有更多的扩展内联的语法，笔者这里偷个懒不多说了，我们操作系统中的使用更多的是执行一些只有汇编才有的指令的时候，使用内联汇编，感兴趣的朋友请参考：

> [GCC 扩展内联汇编简介 - thammer - 博客园](https://www.cnblogs.com/thammer/p/12591383.html)
>
> [Extended Asm (Using the GNU Compiler Collection (GCC))](https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html)

​	所有的代码都在：

> [代码](./inline_asm_code)