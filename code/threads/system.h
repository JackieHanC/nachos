// system.h 
//	All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SYSTEM_H
#define SYSTEM_H

#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "timer.h"
#include "synch.h"

// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	// Initialization,
						// called before anything else
extern void Cleanup();				// Cleanup, called when
						// Nachos is done.

extern void ts();

extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  		// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;			// performance metrics
extern Timer *timer;				// the hardware alarm clock

extern Lock *lock;

#ifdef USER_PROGRAM
#include "machine.h"
extern Machine* machine;	// user program memory and registers
extern int misscnt;
extern int hitcnt;
#endif

#ifdef FILESYS_NEEDED 		// FILESYS or FILESYS_STUB 
#include "filesys.h"
extern FileSystem  *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk   *synchDisk;
#endif

#ifdef NETWORK
#include "post.h"
extern PostOffice* postOffice;
#endif

class message {
public:
    message() {
        msgID = -1;
        senderID = -1;
        receiverID = -1;
        length = -1;
        content = NULL;
        next_message = NULL;
    }
    void init(int _senderID, int _receiverID, char * _content, int _msgID) {
        msgID = _msgID;
        senderID = _senderID;
        receiverID = _receiverID;
        length = strlen(_content);
        if (content != NULL){
            delete content;
        }
        content = new char[length+1];
        strcpy(content, _content);
    }
    message(const message &m) {
        msgID = m.msgID;
        senderID = m.senderID;
        receiverID = m.receiverID;
        length = m.length;
        content = new char[length+1];
        strcpy(content, m.content);
        next_message = NULL;
    }
    ~message() {
        if(content != NULL) {
            delete content;
        }

        // this is wrong, should delete recursively
        // buf I don't want to change it
        if (next_message != NULL) {
            delete next_message;
        }
    }
    int msgID;
    int senderID;
    int receiverID;
    int length;
    char *content;
    message * next_message;
};

class message_buffer {
public:
    message_buffer(int size) {
        max = size;
        buf = new message[max];
        isEmpty = new bool[max];
        for (int i = 0;i < max;++i) {
            isEmpty[i] = false;
        }
        mutex1 = new Semaphore("mutex1", 1);
        mutex2 = new Semaphore("mutex2", 1);
        buf_full = new Semaphore("buf_full", 0);
        buf_empty = new Semaphore("buf_empty", max);
    }
    message * buf;
    bool *isEmpty;
    int max;
    Semaphore * mutex1;
    Semaphore * mutex2;
    Semaphore * buf_full;
    Semaphore * buf_empty;
    void send(int _senderID, int _receiverID, char *_content) {
        if (t_ids[_receiverID] != 1) {
            printf("receiver does not exist.\n");
            return;
        }
        int empty_num = -1;
        buf_empty->P();
        mutex1->P();
        for (int i = 0;i < max;++i) {
            if (isEmpty[i] == false) {
                isEmpty[i] = true;
                empty_num = i;
                break;
            }
        }
        mutex1->V();
        
        buf[empty_num].init(_senderID, _receiverID, _content, empty_num);
        
        mutex2->P();
        Thread* receiverThread = tid_pointer[_receiverID];
        
        if (receiverThread->msgQueue == NULL) {
        	receiverThread->msgQueue = new message(buf[empty_num]);
        }
        else {
        	message *tmp = new message(buf[empty_num]);
        	message * t = receiverThread->msgQueue;
        	while (t->next_message != NULL) {
        		t = t->next_message;
        	}
        	t->next_message = tmp;
        }
        
        mutex2->V();
        buf_full->V();
    }

    void receive(char * target) {
        message * tmp = currentThread->msgQueue;
        if (tmp == NULL) {
            printf("there is no message.\n");
            return;
        }
        buf_full->P();

        mutex2->P();
        target = new char[tmp->length+1];
        strcpy(target, tmp->content);
        currentThread->msgQueue = tmp->next_message;

        mutex2->V();

        mutex1->P();
        isEmpty[tmp->msgID] = false;
        delete tmp;
        mutex1->V();
        buf_empty->V();
    }
};

extern message_buffer msg_buffer;
#endif // SYSTEM_H
