#include "syscall.h"
int main(){
	int id = Exec("halt");
	Join(id);
}