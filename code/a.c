#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
int
OpenForWrite(char *name)
{
    int fd = open(name, O_RDWR|O_CREAT|O_TRUNC, 0666);
    printf("fd is %d\n",fd );
    assert(fd >= 0); 
    return fd;
}
int main(){
    int a = OpenForWrite("a.txt");
    return 0;
}
