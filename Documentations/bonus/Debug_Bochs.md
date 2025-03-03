# 调试我们的操作系统——bochs调试礼记

我们当然要学习如何使用bochs来调试我们的操作系统。毕竟伴随代码量的增大，出错的概率自然也会直线的上升。

## 我们可以调试OS的什么

- 我们可以查看页表，查看GDT,IDT等后面我们编写操作系统会使用到的数据结构
- 可以看到当前线程流的栈的数据
- 可以反汇编查看内存
- 可以实模式保护模式相互转换的时候发生提醒
- 中断发生的时候提醒

## 理解bochs调试的单位内存尺度

​	bochs调试的单位内存尺度是4个字节为一个单位。所以，当我们想要表达，比如说我们反汇编数据的显示格式的时候，可以指定为：

- b: 一个字节大小
- h: 两个字节大小
- w:四个字节大小
- g: 八个缔结大小

​	当然，显示上更是有八进制，十进制，十六进制的数字显示。一般我们会使用十六进制显示/

## 查看内存内容

```
xp /nuf <address>
```

- `n`：要显示的内存单元数量。
- `u`：每个内存单元的大小（b=字节, h=半字, w=字, g=双字）。
- `f`：显示格式（x=十六进制, d=十进制, u=无符号十进制, o=八进制, t=二进制）。
- `<address>`：要查看的内存地址。

​	举一个例子，比如说我们想要看MBR的前十个字节。我们习惯上就会：

```
<bochs:1> xp /10bx 0x7c00
```

​	如果我们在没有运行的时候，就去敲入这个指令，就会发现

```
[bochs]:
0x00007c00 <bogus+       0>:    0x00    0x00    0x00    0x00    0x00    0x00    0x00    0x00
0x00007c08 <bogus+       8>:    0x00    0x00
```

​	这是可以理解的，MBR还没有被加载进来，所以，等到我们运行一下我们的MBR。

```
<bochs:2> c
^CNext at t=30499013
(0) [0x000000000909] 0000:0909 (unk. ctxt): jmp .-2  (0x00000909)     ; ebfe
```

​	敲入C是让操作系统在虚拟机中跑起来，随后敲下Ctrl C后，他会停在我们写的jmp $上，在这里就是成功了。我们这个时候再看

```
[bochs]:
0x00007c00 <bogus+       0>:    0x8c    0xc8    0x8e    0xd8    0x8e    0xc0    0x8e    0xd0
0x00007c08 <bogus+       8>:    0x8e    0xe0
```

​	指令就被加载进来了！

## `disasm` 作为反汇编指令查看我们正在执行的内容

```
disasm [start] [end]
```

- `start`：要反汇编的起始内存地址。
- `end`：要反汇编的结束内存地址（可选）。

​	如果只提供 `start` 参数，Bochs 会从该地址开始反汇编一定数量的指令（通常是 10 条）。如果同时提供 `start` 和 `end` 参数，Bochs 会反汇编从 `start` 到 `end` 之间的所有指令。

​	比如说，我们来从地址 `0x7c00` 开始到`0x7c40`这0x40个字节的反汇编的结果

```
disasm 0x7c00 0x7c80
```

​	你可以看到这样的结果。

```
00007c00: (                    ): mov ax, cs                ; 8cc8
00007c02: (                    ): mov ds, ax                ; 8ed8
00007c04: (                    ): mov es, ax                ; 8ec0
00007c06: (                    ): mov ss, ax                ; 8ed0
00007c08: (                    ): mov fs, ax                ; 8ee0
00007c0a: (                    ): mov sp, 0x7c00            ; bc007c
00007c0d: (                    ): mov ax, 0xb800            ; b800b8
00007c10: (                    ): mov gs, ax                ; 8ee8
```

## 打断点

​	打断点上，分为三种。一种是b，也就是对物理地址打断点，这里，为了给你的操作系统的MBR打断点，你需要

```
b *0x7c00
```

​	这就是在0x7c00上的指令打断点，当我们输入c回车，操作需要跑起来之后，就会停在这里

```
<bochs:1> b *0x7c00
<bochs:2> c
(0) Breakpoint 1, 0x00007c00 in ?? ()
Next at t=17175571
(0) [0x000000007c00] 0000:7c00 (unk. ctxt): mov ax, cs                ; 8cc8
```

​	当然，还有另一个常用的就是启用页表后，我们的地址全部编程了虚拟地址，这个时候，打断点就需要使用vb指令了！这个我会在后面进入保护模式的时候，直接现场演示。

​	另一种打断点的方式是根据CPU的执行方式打断点，比如说

- sb :再执行给定的若干条指令停下，但是笔者更喜欢用的是s + 指令数目
- sba t: 设置在CPU执行虚拟时间为t的时候停下来，这个是笔者自己排查系统调用的bug的时候经常用。

`watch` 命令用于设置读写断点，常用的子命令如下：

- `watch r/read [phy_addr]`：设置读断点。如果物理地址 `phy_addr` 有读操作，则停止运行。
- `watch w/write [phy_addr]`：设置写断点。如果物理地址 `phy_addr` 有写操作，则停止运行。此命令非常有用，如果某块内存不知何时被改写了，可以设置此中断。
- `watch`：显示所有读写断点。
- `unwatch`：清除所有断点。
- `unwatch [phy_addr]`：清除在此地址上的读写断点。

## show int查看中断

​	`show int` 命令用于显示当前中断描述符表（IDT, Interrupt Descriptor Table）的内容。中断描述符表是 x86 架构中的一个重要数据结构，用于管理中断和异常的处理程序。我们实际上用这个看中断描述表IDT的内容，这个是我们为我们的操作系统建立中断的时候才会需要的。

## info

`info` 是一个指令族，执行 `help info` 时可以查看其所有支持的子命令。常用的子命令如下：

- `info pb|break|b`：查看断点信息，等同于 `blist`。
- `info CPU`：显示 CPU 所有寄存器的值，包括不可见寄存器。
- `info fpu`：显示 FPU 状态。
- `info idt`：显示中断向量表 IDT。
- `info gdt [num]`：显示全局描述符表 GDT。如果加了 `num`，只显示 GDT 中第 `num` 项描述符。
- `info ldt`：显示局部描述符表 LDT。
- `info tss`：显示任务状态段 TSS。
- `info ivt [num]`：显示中断向量表 IVT。和 GDT 一样，如果指定了 `num`，则只会显示第 `num` 项的中断向量。
- `info flagsleflags`显示状态寄存器，其实在用r命令显示寄存器值时也会输出eflags的状态，还会输出通用寄存器的值，我通常会用r来看。

## 其他指令

- `print-stack [num]`：显示堆栈，`num` 默认为 16，表示打印的栈条目数。输出的栈内容是栈顶在上，低地址在上，高地址在下。这和栈的实际扩展方向相反，这一点请注意。
- `reg`查看所有的通用寄存器的值
- `sreg` 显示所有段寄存器的值。
- `dreg` 显示所有调试寄存器的值。
- `creg` 显示所有控制寄存器的值。
- `info tab`显示页表中线性地址到物理地址的映射。
- `page line_addr` 显示线性地址到物理地址间的映射。

