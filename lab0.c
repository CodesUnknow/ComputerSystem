/*lab0 操作系统的CODING主要通过C语言实现，LAB0主要是熟悉经常用到的数据结构*/
#include <stdio.h>

#define STS_IG32        0xE            // 32-bit Interrupt Gate
#define STS_TG32        0xF            // 32-bit Trap Gate

typedef unsigned uint32_t;
/*分别对gatedesc结构体中的位进行操作*/
#define SETGATE(gate, istrap, sel, off, dpl) {            \
    (gate).gd_off_15_0 = (uint32_t)(off) & 0xffff;        \
    (gate).gd_ss = (sel);                                \
    (gate).gd_args = 0;                                    \
    (gate).gd_rsv1 = 0;                                    \
    (gate).gd_type = (istrap) ? STS_TG32 : STS_IG32;    \
    (gate).gd_s = 0;                                    \
    (gate).gd_dpl = (dpl);                                \
    (gate).gd_p = 1;                                    \
    (gate).gd_off_31_16 = (uint32_t)(off) >> 16;        \
}

 /* Gate descriptors for interrupts and traps */
/*结构体中冒号的赋值方式，标明了变量所占的位，比如gd_off_15_0这个变量就占了结构体起始的16个位*/
/*这个结构体变量一种占了64位，等于8个字节，这个结构体的sizeof输出也是8*/
 struct gatedesc {
    unsigned gd_off_15_0 : 16;        // low 16 bits of offset in segment
    unsigned gd_ss : 16;            // segment selector
    unsigned gd_args : 5;            // # args, 0 for interrupt/trap gates
    unsigned gd_rsv1 : 3;            // reserved(should be zero I guess)
    unsigned gd_type : 4;            // type(STS_{TG,IG32,TG32})
    unsigned gd_s : 1;                // must be 0 (system)
    unsigned gd_dpl : 2;            // descriptor(meaning new) privilege level
    unsigned gd_p : 1;                // Present
    unsigned gd_off_31_16 : 16;        // high bits of offset in segment
 };
 
int
main(void)
{
    unsigned before;
    unsigned intr;
    unsigned after;
    struct gatedesc gintr;
    
    intr=8;
    before=after=0;
/*gintr的值是以无符号变量intr为起始地址的，它的大小为8个字节（进行了类型转换)*/
    gintr=*((struct gatedesc *)&intr);
    SETGATE(gintr, 0,1,2,3);
    intr=*(unsigned *)&(gintr);
/*intr变量和gintr变量分别输出了4个字节和8个字节，具体字节上的数字，通过setgate这个声明中的操作进行赋值*/
    printf("intr is 0x%x\n",intr);
    printf("gintr is 0x%llx\n",gintr);
    int size = sizeof(gintr);
    printf("size is %d",size);
    
    return 0;
}
