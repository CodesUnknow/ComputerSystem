#EX1,CODE
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

#EX2
#make部分，主要是进入QEMU调试
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
