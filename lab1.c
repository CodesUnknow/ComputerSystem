#EX1,CODE
#操作系统镜像文件ucore.img是如何一步一步生成的？(需要比较详细地解释Makefile中每一条相关命令和命令参数的含义，以及说明命令导致的结果)
#答：通过make V=指令可以看到ucore.img生成的过程，具体就是gcc对c语言进行编译，ld命令将编译文件进行链接，最后生成img文件。
#对于C、C++、pas等，首先要把源文件编译成中间代码文件，Windows下.obj，UNIX下.o，即 Object File，这个动作叫做编译（compile）。
#然后再把大量的Object File合成执行文件，这个动作叫作链接（link）。
#有关makefile的一些补充知识：https://blog.csdn.net/liang13664759/article/details/1771246/
#一个被系统认为是符合规范的硬盘主引导扇区的特征是什么？
#答：根据阅读sign.c源码得知，一个符合规范的硬盘主引导扇区有两个特征，总的大小在512字节之内，最后的两个字节是标志位0x55,0xAA。
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
//判断main函数的argc和argv，argc是传入主函数的参数个数，argv是参数序列，argv[0]是程序的名称以及程序的路径
int
main(int argc, char *argv[]) {
    struct stat st;
    if (argc != 3) {
        fprintf(stderr, "Usage: <input filename> <output filename>\n");
        return -1;
    }
//stat函数引用自sys/stat.h库，通过文件名字获取文件的属性，同时定义了一个文件属性格式的结构体，将这个文件属性拷贝过去，成功则返回为0
    if (stat(argv[1], &st) != 0) {
        fprintf(stderr, "Error opening file '%s': %s\n", argv[1], strerror(errno));
        return -1;
    }
    printf("'%s' size: %lld bytes\n", argv[1], (long long)st.st_size);
//不能超过512字节，实际内容不能超过510字节，因为最后两个字节是标志位，
    if (st.st_size > 510) {
        fprintf(stderr, "%lld >> 510!!\n", (long long)st.st_size);
        return -1;
    }
    char buf[512];
    memset(buf, 0, sizeof(buf));
    FILE *ifp = fopen(argv[1], "rb");
    int size = fread(buf, 1, st.st_size, ifp);
    if (size != st.st_size) {
        fprintf(stderr, "read '%s' error, size is %d.\n", argv[1], size);
        return -1;
    }
    fclose(ifp);
    buf[510] = 0x55;
    buf[511] = 0xAA;
    FILE *ofp = fopen(argv[2], "wb+");
    size = fwrite(buf, 1, 512, ofp);
    if (size != 512) {
        fprintf(stderr, "write '%s' error, size is %d.\n", argv[2], size);
        return -1;
    }
    fclose(ofp);
    printf("build 512 bytes boot sector: '%s' success!\n", argv[2]);
    return 0;
}
/*
#EX2
#make部分，主要是进入QEMU调试
1、从CPU加电后执行的第一条指令开始，单步跟踪BIOS的执行。
时钟无法正常加载？第一条指令不是长跳转应有的地址？
2、在初始化位置0x7c00设置实地址断点,测试断点正常。
这段代码在lab1init中，b *0x7c00部分

3、从0x7c00开始跟踪代码运行,将单步跟踪反汇编得到的代码与bootasm.S和 bootblock.asm进行比较。
相同
4、自己找一个bootloader或内核中的代码位置，设置断点并进行测试。
如何在源代码中得到这条指令编译后的地址？

lab1-mon: $(UCOREIMG)
	$(V)$(TERMINAL) -e "$(QEMU) -S -s -d in_asm -D $(BINDIR)/q.log -monitor stdio -hda $< -serial null"
	$(V)sleep 2
	$(V)$(TERMINAL) -e "gdb -q -x tools/lab1init"
#同时在lab1init里面进行了初始化的设置
#包括内核，QEMU的端口，架构模式，以及断点，语法依然看不太懂
file bin/kernel
target remote :1234
set architecture i8086
b *0x7c00
continue
x /2i $pc
*/

/*
#EX3
BIOS将通过读取硬盘主引导扇区到内存，并转跳到对应内存中的位置执行bootloader。请分析bootloader是如何完成从实模式进入保护模式的。
需要掌握的一部分关于GDT,LDT的先验知识：http://www.techbulo.com/708.html
需要掌握：为何开启A20，以及如何开启A20；
答：打开A20后，操作系统会从实模式切换到保护模式，打开前只能读取1MB的内存，超过会回卷。打开方式为：设置0x64；0x60两个IO端口特定值
如何初始化GDT表；
答：利用lgdt命令将GDT的大小和起始地址加载到GDT寄存器
补充知识：
全局描述符表GDT（Global Descriptor Table）在整个系统中，全局描述符表GDT只有一张(一个处理器对应一个GDT)，GDT可以被放在内存的任何位置，
但CPU必须知道GDT的入口，也就是基地址放在哪里，Intel的设计者门提供了一个寄存器GDTR用来存放GDT的入口地址，程序员将GDT设定在内存中某个位置之后，
可以通过LGDT指令将GDT的入口地址装入此寄存器，从此以后，CPU就根据此寄存器中的内容作为GDT的入口来访问GDT了。
GDTR中存放的是GDT在内存中的基地址和其表长界限。
基地址指定GDT表中字节0在线性地址空间中的地址，表长度指明GDT表的字节长度值。指令LGDT和SGDT分别用于加载和保存GDTR寄存器的内容。
在机器刚加电或处理器复位后，基地址被默认地设置为0，而长度值被设置成0xFFFF。在保护模式初始化过程中必须给GDTR加载一个新值。

如何使能和进入保护模式
答：读代码部分，A20
代码如下：
#include <asm.h>

# Start the CPU: switch to 32-bit protected mode, jump into C.
# The BIOS loads this code from the first sector of the hard disk into
# memory at physical address 0x7c00 and starts executing in real mode
# with %cs=0 %ip=7c00.

.set PROT_MODE_CSEG,        0x8                     # kernel code segment selector
.set PROT_MODE_DSEG,        0x10                    # kernel data segment selector
.set CR0_PE_ON,             0x1                     # protected mode enable flag

# start address should be 0:7c00, in real mode, the beginning address of the running bootloader
# 初始化过程，因此1、屏蔽中断；2、采用CLD模式的地址读取方式（低地址到高地址）
.globl start
start:
.code16                                             # Assemble for 16-bit mode
    cli                                             # Disable interrupts
    cld                                             # String operations increment

    # Set up the important data segment registers (DS, ES, SS).
    #初始化段寄存器，因为ucore采用段寄存器为0的+偏移地址的方式，所以将ax清零以后，用ax值初始化其他的段寄存器
    xorw %ax, %ax                                   # Segment number zero
    movw %ax, %ds                                   # -> Data Segment
    movw %ax, %es                                   # -> Extra Segment
    movw %ax, %ss                                   # -> Stack Segment

    # Enable A20:
    #  For backwards compatibility with the earliest PCs, physical
    #  address line 20 is tied low, so that addresses higher than
    #  1MB wrap around to zero by default. This code undoes this.
    #inb从0x64端口（IO设备）读取一个字节，等待IO设备不再繁忙
    #至于为什么是这两个端口，可以参考课程的A20附录部分
    #向0x64端口写入0xd1
    #向0x60端口写入0xdf
seta20.1:
    inb $0x64, %al                                  # Wait for not busy(8042 input buffer empty).
    testb $0x2, %al
    jnz seta20.1
    #向这个端口写入0xd1
    movb $0xd1, %al                                 # 0xd1 -> port 0x64
    outb %al, $0x64                                 # 0xd1 means: write data to 8042's P2 port

seta20.2:
    inb $0x64, %al                                  # Wait for not busy(8042 input buffer empty).
    testb $0x2, %al
    jnz seta20.2

    movb $0xdf, %al                                 # 0xdf -> port 0x60
    outb %al, $0x60                                 # 0xdf = 11011111, means set P2's A20 bit(the 1 bit) to 1

    # Switch from real to protected mode, using a bootstrap GDT
    # and segment translation that makes virtual addresses
    # identical to physical addresses, so that the
    # effective memory map does not change during the switch.
    #以下是加载全局描述符部分，lgdt对应的对全局表寄存器的操作，用这个命令来加载GDT的大小和起始地址到GDT寄存器中
    #其中的数值在gdtdesc中，其中word为大小，long为地址
    lgdt gdtdesc
    movl %cr0, %eax
    orl $CR0_PE_ON, %eax
    movl %eax, %cr0

    # Jump to next instruction, but in 32-bit code segment.
    # Switches processor into 32-bit mode.
    ljmp $PROT_MODE_CSEG, $protcseg

.code32                                             # Assemble for 32-bit mode
protcseg:
    # Set up the protected-mode data segment registers
    movw $PROT_MODE_DSEG, %ax                       # Our data segment selector
    movw %ax, %ds                                   # -> DS: Data Segment
    movw %ax, %es                                   # -> ES: Extra Segment
    movw %ax, %fs                                   # -> FS
    movw %ax, %gs                                   # -> GS
    movw %ax, %ss                                   # -> SS: Stack Segment

    # Set up the stack pointer and call into C. The stack region is from 0--start(0x7c00)
    设置完成后，跳转到bootmain(C语言）
    movl $0x0, %ebp
    movl $start, %esp
    call bootmain

    # If bootmain returns (it shouldn't), loop.
spin:
    jmp spin

# Bootstrap GDT
.p2align 2                                          # force 4 byte alignment
gdt:
    SEG_NULLASM                                     # null seg
    SEG_ASM(STA_X|STA_R, 0x0, 0xffffffff)           # code seg for bootloader and kernel
    SEG_ASM(STA_W, 0x0, 0xffffffff)                 # data seg for bootloader and kernel

gdtdesc:
    .word 0x17                                      # sizeof(gdt) - 1
    .long gdt                                       # address gdt
*/

/*
#EX4
通过阅读bootmain.c，了解bootloader如何加载ELF文件。通过分析源代码和通过qemu来运行并调试bootloader&OS，
这里就涉及到下面两个关键的问题，如何读取硬盘的扇区，如何识别出ELF格式的文件（ucore操作系统的格式是ELF格式的）
在main函数中，通过readseg（通过读取elf文件的header进行判断）\readsect（读扇区，需要大概了解从哪读，读多大）来具体实现
bootloader如何读取硬盘扇区的？
答：从代码初步分析是从0x10000这个地址，每次读一个扇区的大小，然后判断扇区的格式，是否是ELF，这个是通过对文件头特定字节的判断
bootloader是如何加载ELF格式的OS？
bootloader的访问硬盘都是LBA模式的PIO（Program IO）方式，即所有的IO操作是通过CPU访问硬盘的IO地址寄存器完成。
一般主板有2个IDE通道，每个通道可以接2个IDE硬盘。访问第一个硬盘的扇区可设置IO地址寄存器0x1f0-0x1f7实现的，具体参数见下表。
第一个IDE通道通过访问IO地址0x1f0-0x1f7来实现，第二个IDE通道通过访问0x170-0x17f实现。
每个通道的主从盘的选择通过第6个IO偏移地址寄存器来设置。
答：直接跳转到ELF格式文件的入口处
提示：可阅读“硬盘访问概述”，“ELF执行文件格式概述”这两小节。
*/
练习5：实现函数调用堆栈跟踪函数 （需要编程）
我们需要在lab1中完成kdebug.c中函数print_stackframe的实现，可以通过函数print_stackframe来跟踪函数调用堆栈中记录的返回地址。
在如果能够正确实现此函数，可在lab1中执行 “make qemu”后，在qemu模拟器中得到类似如下的输出：
……
ebp:0x00007b28 eip:0x00100992 args:0x00010094 0x00010094 0x00007b58 0x00100096
    kern/debug/kdebug.c:305: print_stackframe+22
ebp:0x00007b38 eip:0x00100c79 args:0x00000000 0x00000000 0x00000000 0x00007ba8
    kern/debug/kmonitor.c:125: mon_backtrace+10
ebp:0x00007b58 eip:0x00100096 args:0x00000000 0x00007b80 0xffff0000 0x00007b84
    kern/init/init.c:48: grade_backtrace2+33
ebp:0x00007b78 eip:0x001000bf args:0x00000000 0xffff0000 0x00007ba4 0x00000029
    kern/init/init.c:53: grade_backtrace1+38
ebp:0x00007b98 eip:0x001000dd args:0x00000000 0x00100000 0xffff0000 0x0000001d
    kern/init/init.c:58: grade_backtrace0+23
ebp:0x00007bb8 eip:0x00100102 args:0x0010353c 0x00103520 0x00001308 0x00000000
    kern/init/init.c:63: grade_backtrace+34
ebp:0x00007be8 eip:0x00100059 args:0x00000000 0x00000000 0x00000000 0x00007c53
    kern/init/init.c:28: kern_init+88
ebp:0x00007bf8 eip:0x00007d73 args:0xc031fcfa 0xc08ed88e 0x64e4d08e 0xfa7502a8
<unknow>: -- 0x00007d72 –
……
编程的要求不高，但是怎么执行呢 -。-...如果是直接写到main函数中实现？那么之前的main函数会直接跳转到操作系统的入口处，已将权限交给了操作系统。
