#include "syscall.h"

int fd;
int byteNum;
char content[20];
int main()
{

    Create("w.txt");
    fd = Open("w.txt");
    Write("hello world", 11, fd);
    byteNum = Read(content, 11, fd);
    Close(fd);
    Halt();
}
