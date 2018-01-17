#include "syscall.h"
int main()
{
	Exec("halt");
	Yield();
}