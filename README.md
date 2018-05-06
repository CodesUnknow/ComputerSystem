# ComputerSystem
learning cs from the code view
关于清华大学操作系统的学习笔记，实验指导书以及源代码地址：https://objectkuan.gitbooks.io/ucore-docs/content/
#Lab0
掌握最基本的工具和操作系统中常用的数据结构，比较简单，用到了gcc等编译工具，需要一些C语言基础。具体代码参考Lab0.c
#Lab1
实验目的：
Lab 1主要是关于操作系统如何启动，以及如何去和中断函数调用栈相关的一些知识
Lab1之前的理论知识储备
•	1、操作系统的启动过程
•	系统加电如何启动到uCore操作系统去呢。具体过程为：首先计算机加电以后，会有一个初始状态，所有的寄存器都会有一个初始的缺省值。其中与启动关系最为密切的就是段地址寄存器CS以及指针指令寄存器EIP，CS和EIP的数值结合起来就是第一条指令的地址（启动地址）。其它还有一些标志位等等也很重要，比如EFLAGS 前面介绍的标志位，还有一些控制寄存器比如说我们后面会讲到CR0控制寄存器。
•	具体的实例：CS在初始的时候是F000，EIP是32位，值为FFF0，需要注意的是：X86一开始加电时候 启动实模式，这个向下兼容的实模式只支持20位的地址总线，因此只能寻址1MB。CS是段寄存器，段寄存器里面有隐含Base内容，Base代表基址FFFF0000。再加上刚才的EIP是FFF0，所以说它们启动最终的一个地址是FFFFFFF0这个地址，这个地址，其实就是我们加电之后，要去取得那个内存所在的地址。然后计算机会根据CS和EIP的值进行寻址，对于寄存器中的具体指，CS会向左移4位，再与EIP相加，最终形成了20位的地址总线。那么这个起始地址存的是什么呢？在PC中的一个固件叫EPROM ，初始指令会跳转到EPROM所在内存区域中的一个地址（这个地址的内容是只读的），从该地址会取得第一条长跳转指令，跳到BIOS中做初始化工作，从这个特殊的地址跳到一个可以被访问的1M的内存空间里面去执行。
2、BIOS完成的工作：
•	底层的硬件初始化工作
•	各种外设 CPU内存的质检
•	完成后，这个固件会去加载磁盘或者硬盘的第一个主引导扇区，这个主引导扇区是零号扇区，一个扇区的内容是512个字节，加载到固定地址 0X7C00处，这个内容就是一个bootloader。它完成来对我们说的这个操作系统 uCore的进一步加载。
•	主引导扇区里面存着Bootloader的代码，那么它要干什么事情呢，1、实模式切换到保护模式，从实模式的16位的寻址空间切换到了32位的寻址空间，从1M的寻址到了4G的寻址，一旦Enable（使能）了这个保护模式。也就意味着这个所谓的段机制也就自动的加载上来了；2、读取kernel 就是uCore的代码，又从哪读呢 也是一样 存在我们的硬盘中，它需要从硬盘里面把uCore的代码，再从我们的硬盘的扇区里面读到内存中来
3、X86操作系统的实模式和保护模式
•	实模式 保护模式有它的不同的特征，为什么一开始的实模式，后面又进入了保护模式。在保护模式下干什么事情以及去理解一旦进入保护模式之后，段机制是怎么一回事。
•	保护模式下，功能的变化，首先使能了段机制，（段大小的限制和优先级问题，段机制是页机制的基础），实现段机制有两个重要的指标，段的起始地址和大小，因此段模式对应的内存空间是线性的，实现起来也需要一个大的数组，将各个段的段描述符装进去（段描述符一般用来储存段机制的指标）。这个数组称为全局描述符表（GDT），GDT是由Bootloader来建立的，GDT可以让CPU能够找到段表的起始地址，然后通过GDTR这个寄存器来保存相应的地址信息。
•	在UCORE中，这个映射被弱化了，段地址的基址都是零，长度都是4G，相当于物理寻址。在LAB2中，会讲到UCORE实际上使用页机制来实现分段的功能。
•	段寄存器的格式：段寄存器存放的是段描述符，包括段落的起始地址和大小。具体来讲：段寄存器一部分位会存储一个INDEX（段选择子），通过映射表，会得到这个INDEX对应的物理地址以及它的大小，结合映射表以及EIP（指针寄存器）存储的offset，就可以找到对应的物理内存了（在没有页机制的情况下）。
•	具体的位数划分，以一个16位的段寄存器为例，高13位放的就是GDT的Index，接下来的两位RP，表明这个段当前的段的优先级的级别，在X86里面 它用了两个bit来表示这个优先级别，意味着它可以表示0 1 2 3四个特权级。一般说我们操作系统放在最高的级别是0特权级，而我们的应用程序会放在3这个特权级里面。
以上，我们就拥有了建立段机制的所有条件，还需要一点点工作，就是在实模式加载bootloader以后，要执行一个实模式到保护模式的切换过程，这个功能是通过一个特定的寄存器，系统寄存器CRO来实现的，将CRO的第0号位设置为1，操作系统就进入了保护模式。

Lab1-EX1实验的两个目的：
1、操作系统镜像文件ucore.img是如何一步一步生成的？(需要比较详细地解释Makefile中每一条相关命令和命令参数的含义，以及说明命令导致的结果)
2、一个被系统认为是符合规范的硬盘主引导扇区的特征是什么？
1、用make V=命令会将make指令中的每一步操作逐步显示出来，大概会先调用GCC，生成.o文件，然后生成.out执行文件，后续还会创建一个虚拟硬盘，将ucore.img文件（即操作系统的内核）创建在这里。
2、这个规范使用sign.c文件中的代码进行规范，具体研读代码。

Lab1-EX2实验的目的：
熟悉使用qemu和gdb进行的调试工作，具体的步骤包括：
从CPU加电后执行的第一条指令开始，单步跟踪BIOS的执行。
在初始化位置0x7c00设置实地址断点,测试断点正常。
从0x7c00开始跟踪代码运行,将单步跟踪反汇编得到的代码与bootasm.S和 bootblock.asm进行比较。
自己找一个bootloader或内核中的代码位置，设置断点并进行测试。
实际练习中，实验楼启动QEMU后会非常慢，使用make lab1-mon就可以对这部分源代码进行执行，这段代码初始化QEMU的一些初始设置，并将PC跳转到0X7C00，此时加载了BOOTLOADER，然后就可以在QUEMU中进行GDB调试了。其中bootloader部分的源码为bootasm.S以及bootmain.c

Lab1-EX3实验目的：
BIOS将通过读取硬盘主引导扇区到内存，并转跳到对应内存中的位置执行bootloader。请分析bootloader是如何完成从实模式进入保护模式的。
需要阅读小节“保护模式和分段机制”和lab1/boot/bootasm.S源码，了解如何从实模式切换到保护模式，需要了解：为何开启A20，以及如何开启A20；如何初始化GDT表；如何使能和进入保护模式
