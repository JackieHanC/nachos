// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "thread.h"
#include "sysdep.h"
// #include <stdlib.h>

// extern int t_id[128];

void exec(char *name) {
    // printf("exec syscall func\n");
    
    OpenFile * exe = fileSystem->Open(name);
    // printf("open file name %s\n", name);
    AddrSpace *space;

    if (exe == NULL) {
        printf("%s: command not found\n", name);
        return;
    }
    
    space = new AddrSpace(exe);

    currentThread->space = space;
    space->InitRegisters();     
    space->RestoreState(); 
    delete exe;
    machine->Run();
}

void cat(char *filename) {
    OpenFile *f = fileSystem->Open(filename);
    char data;
    while(f->Read(&data, 1)!=0){
        printf("%c",data);    
    }
    printf("\n");
    
}

void rm(char *filename) {
    fileSystem->Remove(filename);
}

void touch(char *filename) {
    fileSystem->Create(filename, 0, 0);
}

void help() {
    printf("Nachos shell version 1.0.\n");
    printf("Supported commands are as follow:\n");
    printf("help\t\t: list usage of this shell.\n");
    printf("ls\t\t: list file or directory of current directory.\n");
    printf("ls [dir]\t: list file or directory of directory [dir].\n");
    printf("cd [dir]\t: change your directory to directory [dir].\n");
    printf("mkdir [dir]\t: create new directory under current directory.\n");
    printf("cat [filename]\t: print content of [filename].\n");
    printf("touch [filename]: create new file named [filename] under current directory.\n");
    printf("rm [filename]\t: remove file named [filename]\n");
    printf("[executable]\t: run your executable file\n");
    printf("quit\t\t: quit the shell.\n");
}

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    if ((which == SyscallException) && (type == SC_Halt)) {
		DEBUG('a', "Shutdown, initiated by user program.\n");

		// printf("misscnt is %d, hit cnt is %d, hitrate is %f\n",
		// 	misscnt,hitcnt, float(hitcnt)/(misscnt+hitcnt));
  		interrupt->Halt();
    }
    else if ((which == SyscallException) && (type == SC_Exit)) {
        printf("Program Exit\n");
        for(int i = 0;i < machine->pageTableSize;i++){
            //int cur = machine->pageTable[i].physicalPage;
            if (!machine->pageTable[i].valid)
                continue;

            if (machine->pageTable[i].threadID == currentThread->getThreadID()) {
                if (machine->bitmap->Test(i)) {
                    machine->bitmap->Clear(i, TRUE);
                    machine->pageTable[i].valid = FALSE;
                }
            }
        }
        
        int NextPC = machine->ReadRegister(NextPCReg);
        machine->WriteRegister(PCReg, NextPC);

        currentThread->Finish();
    }
    else if ((which == SyscallException) && (type == SC_Create)) {
        printf("system call create\n");
        int address = machine->ReadRegister(4);
        char name[10];
        int pos = 0;
        int data;

        while(TRUE) {
            while(!machine->ReadMem(address + pos,1, &data));
            if (data == 0) {
                name[pos] = '\0';
                break;
            }
            name[pos++] = char(data);
        }
        fileSystem->Create(name, 128, 1);
        machine->PC_advance();
    }
    else if ((which == SyscallException) && (type == SC_Open)) {
        printf("system call Open\n");
        int address = machine->ReadRegister(4);
        char name[10];
        int pos = 0;
        int data;
        while(TRUE) {
            while(!machine->ReadMem(address + pos,1, &data));
            if (data == 0) {
                name[pos] = '\0';
                break;
            }
            name[pos++] = char(data);
        }
        OpenFile * openfile = fileSystem->Open(name);
        machine->WriteRegister(2, int(openfile));
        machine->PC_advance();
    }
    else if ((which == SyscallException) && (type == SC_Close)) {
        printf("system call close\n");
        int fd = machine->ReadRegister(4);
        OpenFile *openfile = (OpenFile*)fd;
        delete openfile;
        machine->PC_advance();
    }
    else if ((which == SyscallException) && (type == SC_Read)) {
        // printf("system call read\n");
        int base_postion = machine->ReadRegister(4);
        int count = machine->ReadRegister(5);
        int fd = machine->ReadRegister(6);
        if (fd == 0) {
            for(int i = 0;i < count;i++){
                
                while(!machine->WriteMem(base_postion + i, 1, getchar()));  
            }
            
        }else {
            OpenFile *openfile = (OpenFile*)fd;
            char content[count];
            int result = openfile->Read(content, count);
            for (int i = 0;i < result;++i) 
                while(!machine->WriteMem(base_postion + i, 1, int(content[i])));
            machine->WriteRegister(2, result);
        }
        machine->PC_advance();
    }
    else if ((which == SyscallException) && (type == SC_Write)) {
        // printf("system call write\n");
        int base_postion = machine->ReadRegister(4);
        int count = machine->ReadRegister(5);
        int fd = machine->ReadRegister(6);
        // printf("base_postion %d, count %d, fd %d\n",base_postion, count, fd );
        char content[count];
        int data;
        for (int i = 0;i < count; i++) {
            while(!machine->ReadMem(base_postion + i, 1, &data));
            content[i] = (char)data;
        
        }
        if(fd == 1){
            for(int i = 0;i < count;++i) {
                putchar(content[i]);
            }
        } else{
            OpenFile *openfile = (OpenFile*)fd;
            openfile->Write(content, count);
        }
        machine->PC_advance();


    }

    else if ((which == SyscallException) && (type == SC_Exec)) {
        // printf("system call exec\n");
        int address = machine->ReadRegister(4);
        char *name = new char[10];

        int pos = 0;
        int data;

        while(TRUE) {
            while(!machine->ReadMem(address + pos,1, &data));
            if (data == 0) {
                name[pos] = '\0';
                break;
            }
            name[pos++] = char(data);
        }

        char *name0 = new char[10];
        char *name1 = new char[10];
        int len = strlen(name);
        int firstSpace = -1, lastSpace = -1;
        int flag = 0;
        for (int i = 0;i < len;++i) {
            if (name[i] == ' ') {
                if (flag == 0) {
                    firstSpace = i;
                }
                flag = 1;
                lastSpace = i;

            }
            
        }
        int j;
        for (j = 0;j < firstSpace;++j) {
            name0[j] = name[j];
        }
        name0[j] = '\0';
        for (j = lastSpace + 1; j < len;++j) {
            name1[j - lastSpace - 1] = name[j];
        }
        name1[j - lastSpace - 1] = '\0';

        // command without space
        if (firstSpace == -1) {
            if (strcmp(name, "quit") == 0) {
                interrupt->Halt();
            }
            else if (strcmp(name, "help") == 0) {
                help();
                machine->PC_advance();
            }
            else if (strcmp(name, "ls") == 0) {
                system("ls");
                machine->PC_advance();
            }
            else {
                Thread *new_thread = Thread::getInstance("new thread", 1);

                new_thread->Fork(exec, name);
                machine->WriteRegister(2, new_thread->getThreadID());
                

                machine->PC_advance();
            }
        } else {
            if (strcmp(name0, "ls") == 0) {
                system(name);
                machine->PC_advance();
            }
            else if (strcmp(name0, "cd") == 0) {
                chdir(name1);
                machine->PC_advance();
            }
            else if (strcmp(name0, "cat") == 0) {
                cat(name1);
                machine->PC_advance();
            }
            else if (strcmp(name0, "rm") == 0) {
                rm(name1);
                machine->PC_advance();
            }
            else if (strcmp(name0, "touch") == 0) {
                touch(name1);
                machine->PC_advance();
            }
            else if (strcmp(name0, "mkdir") == 0) {
                system(name);
                machine->PC_advance();
            }
        }
    }
    else if ((which == SyscallException) && (type == SC_Fork)) {
        printf("system call Fork\n");
        int function_pc = machine->ReadRegister(4);
        currentThread->Fork(function_pc, 1);
        machine->PC_advance();
    }

    else if ((which == SyscallException) && (type == SC_Yield)) {
        printf("system call Yield\n");
        machine->PC_advance();
        currentThread->Yield();
    }
    else if ((which == SyscallException ) && (type == SC_Join)) {
        // printf("system call Join\n");
        int threadID = machine->ReadRegister(4);
        while(t_ids[threadID])
            currentThread->Yield();
        machine->PC_advance();
    }
    else if (which == PageFaultException) {
        char filename[10];
    	// tlb miss
    	if (machine->tlb != NULL) {
    		DEBUG('a', "PageFaultException tlb miss");
    		int vpn = (unsigned) machine->registers[BadVAddrReg]/PageSize;
    		int position = -1;
    		for (int i = 0;i < TLBSize;i++){
    			if (machine->tlb[i].valid == FALSE) {
    				position = i;
                    machine->LRUInsert(i);
    				break;
    			}
    		}
            if (position == -1) {
                position = machine->LRUFind();
                machine->LRUHit(position);
            }

    		// if (position == -1) {
    		// 	DEBUG('a', "No empty TLB entry");
    		// 	position = TLBSize - 1;
    		// 	for (int i = 0;i < TLBSize - 1;i++){
    		// 		machine->tlb[i] = machine->tlb[i + 1];
    		// 	}
    		// }
    		machine->tlb[position].valid = TRUE;
    		machine->tlb[position].virtualPage = vpn;
    		machine->tlb[position].physicalPage = machine->pageTable[vpn].physicalPage;
    		machine->tlb[position].use = FALSE;
    		machine->tlb[position].dirty = FALSE;
    		machine->tlb[position].readOnly = FALSE;
    	}else {
    		//page fault
            // printf("Page Fault threadID is %d\n", currentThread->getThreadID());
            
            // printf("BadVAddrReg %d\n",machine->registers[BadVAddrReg] );
            int vpn = (unsigned)machine->registers[BadVAddrReg]/PageSize;
            int position = machine->bitmap->Find();
            if (position == -1) {
                //random replace solution
                int j = Random() % machine->pageTableSize;
                position = machine->pageTable[j].physicalPage;
                if (machine->pageTable[j].dirty == TRUE) {
                    sprintf(filename, "VM%d", machine->pageTable[j].threadID);
                    OpenFile *file = fileSystem->Open(filename);
                    file->WriteAt(&(machine->mainMemory[position*PageSize]),
                        PageSize, machine->pageTable[j].virtualPage * PageSize);

                    delete file;

                }

            }
            
            sprintf(filename, "VM%d", currentThread->getThreadID());
            OpenFile *file = fileSystem->Open(filename);
            if (file == NULL) {
                printf("%s open error\n", filename);
                ASSERT(FALSE);
            }

            file->ReadAt(&(machine->mainMemory[position*PageSize]), PageSize, vpn*PageSize);
            machine->pageTable[position].valid = TRUE;
            machine->pageTable[position].physicalPage =  position;
            machine->pageTable[position].virtualPage = vpn;
            machine->pageTable[position].use = FALSE;
            machine->pageTable[position].dirty = FALSE;
            machine->pageTable[position].readOnly = FALSE;
            machine->pageTable[position].threadID = currentThread->getThreadID();
            delete file;
    	}
    } 
    else {
		printf("Unexpected user mode exception %d %d\n", which, type);
		ASSERT(FALSE);
    }
}
