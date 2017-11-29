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
//#include <stdlib.h>

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

		printf("misscnt is %d, hit cnt is %d, hitrate is %f\n",
			misscnt,hitcnt, float(hitcnt)/(misscnt+hitcnt));
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
            printf("Page Fault threadID is %d\n", currentThread->getThreadID());
            
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
