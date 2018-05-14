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

关于内存管理的基本知识（参考视频的第五章节部分）
1、为什么要用到逻辑地址？
计算机的地址总线决定了物理寻址的空间，比如32位的地址总线，对应了4G的物理空间，相应的编号呢也就是从0到它的最大编号，物理地址对于程序员来说是难以直接利用的，因为在程序运行之前，程序员并不知道内存中哪些地址可用哪些不可用。为了方便的使用存储空间，程序员一般使用逻辑地址。
逻辑地址是进程可以使用的地址空间，当程序加载到内存中运行的时候，操作系统为程序分配的内存中的空间，对应着这段进程的逻辑地址。

2、生成地址的几种情况？
这里的地址指的是物理地址：
1、程序编写完成，在编译的时候直接生成物理地址，比如非智能手机，或者其他程序已经写死的电器；
2、在加载程序的时候，通过重定位，重新编辑逻辑地址，称为物理地址，此时应用程序中存有重定位表；
3、执行的时候，才确定物理地址。这种情形出现在使用虚拟存储的系统中，相对于前两种情形，这种方式相对灵活，

3、程序符号是如何转换到总线上的物理地址的？ 
高级语言与逻辑地址、物理地址之间的关系，高级语言需要进行编译，称为机器语言，同时也需要编译器进行链接，将函数库、或者其他模块中不同地址的内容链接起来，以便于统一分配物理地址。链接会生成一个线性的序列，对应不同模块的地址。
操作系统的重定位功能用于完成逻辑地址首字段与物理地址之间对应关系。

4、连续内存分配算法，以及可能造成的问题？
在分配内存空间时，如果没有其他任何技术支持，那么只能分配一个连续的内存空间，连续内存分配算法是指给进程分配一块不小于指定大小的物理连续的内存区域。
在多个进程不断的分配和回收过程中，连续内存分配会产生碎片问题，碎片问题又分为内碎片和外碎片，内碎片是基于硬件特性的，比如想要一个510字节的空间，但是只能分配512这样的整数字节，从而产生了2字节的碎片空间；外碎片是由于之前回收的进程空间，不可能完全的匹配新的空间申请需求，多出的部分就形成了外碎片。

5、动态分区分配，以及几种算法？
动态分区分配，是按照用户指定的大小进行空间的分配，此时，操作系统需要维系两个数据结构，已分配的分区和空闲分区。
动态分配分区的几种策略：最先匹配、最佳匹配、最差匹配，其中第一种是按顺序，后面两种是按照空闲空间的大小进行匹配。三种策略的优缺点，以及分别需要维护的数据结构。需要注意的是，这几种策略没有最优或者最差，都可以找到最适应的情形。

6、碎片整理需要哪些技术的支持？
首先是重定位，碎片整理相应会产生物理地址空间的重新分配，对于仍然在运行的进程，就要进行重定位。
内存分区对换，将处于等待状态的进程从内存移动到外存中，进而获得更大的可使用空间。在Linux或者Unix系统里有一个分区叫对换区，这个对换区在早期的时候，就是一种充分利用内存的做法。这个交替它的开销是非常大的，原因在于内存和外存之间的速度差的很远。


