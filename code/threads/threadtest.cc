// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "elevatortest.h"


// testnum is set in main.cc
int testnum = 3;

//extern static Thread* Thread::getInstance(char * threadName);

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

// void
// SimpleThread(int which)
// {
//     int num;
    
//     for (num = 0; num < 20; num++) {
// 	    printf("*** thread %d looped %d times\n", which, num);
//         interrupt->SetLevel(IntOn);
//         interrupt->SetLevel(IntOff);
//     }

// }

// //----------------------------------------------------------------------
// // ThreadTest1
// // 	Set up a ping-pong between two threads, by forking a thread 
// //	to call SimpleThread, and then calling SimpleThread ourselves.
// //----------------------------------------------------------------------

// void
// ThreadTest1()
// {
//     DEBUG('t', "Entering ThreadTest1");

//     currentThread->setPriority(1);
    
    

//     Thread *t = Thread::getInstance("forked thread 1", 0, 100);
//     t->Fork(SimpleThread, (void*)1);
//     Thread *t1 = Thread::getInstance("forked thread 2", 0, 200);
//     t1->Fork(SimpleThread, (void*)2);

//     SimpleThread(0);

    

    
//     // Thread *t2 = Thread::getInstance("forked thread 2", 1);
//     // t->Fork(SimpleThread, (void*)2);
    
// }
// void
// SimpleThread1(int n) {
//     while(currentThread->getTimeNeed() > 0) {
//         printf("*** thread %d has  %d time to run, time slice is %d\n",
//             n, currentThread->getTimeNeed(), currentThread->getuseTime());
//         interrupt->SetLevel(IntOff);
//         interrupt->SetLevel(IntOn);
//     }
// }

// void 
// ThreadTest2() {
//     DEBUG('t', "Entering ThreadTest2");
//     currentThread->setPriority(0);
//     Thread *t = Thread::getInstance("forked thread 1", 0, 100);
//     t->Fork(SimpleThread1,(void*)1);
//     Thread *t1 = Thread::getInstance("forked thread 2", 0, 200);
//     t1->Fork(SimpleThread1,(void*)2);

// }

// void SimpleThreadForLock(int which)
// {
//     lock->Acquire();
//     printf("Running Thread %d which priority is %d\n",which,currentThread->getPriority());
//     lock->Release();
// }

// void 
// ThreadTestForLock()
// {
//     DEBUG('t', "Entering ThreadTestForLock");
    
//     currentThread->setPriority(1);
//     lock->Acquire();
//     Thread *t = Thread::getInstance("forked thread 1", 0);
//     t->Fork(SimpleThreadForLock,(void*)1);

//     printf("After fork thread 1\n");
//     lock->Release();
// }

// class sbuf {
// public:
//     int buf;
//     int n;
//     Semaphore *slots;
//     Semaphore *items;
//     Semaphore *mutex;
//     sbuf(int n) {
//         this->n = n;
//         buf = 0;
//         mutex = new Semaphore("mutex", 1);
//         items = new Semaphore("items", 0);
//         slots = new Semaphore("slots", n);
//     }
//     ~sbuf() {
//         delete mutex;
//         delete items;
//         delete slots;
//     }

//     void insert() {
//         slots->P();
//         mutex->P();
//         buf++;
//         printf("%s Operation insert. items num is %d\n",
//             currentThread->getName(),buf);
//         mutex->V();
//         items->V();
//     }

//     void remove() {
//         items->P();
//         mutex->P();
//         buf--;
//         printf("%s Operation remove. items num is %d\n",
//             currentThread->getName(),buf);
//         mutex->V();
//         slots->V();
//     }
// };
// sbuf *p_c_buf; 
// void  producer(void * which) {
//     while(currentThread->getTimeNeed() > 0) {
//         interrupt->OneTick();
//         p_c_buf->insert();
//     }
// }

// void consumer(void * which) {
//     while(currentThread->getTimeNeed() > 0) {
//         interrupt->OneTick();
//         p_c_buf->remove();
//     }
// }

// void ThreadTestP_C() {
//     p_c_buf = new sbuf(10);

//     Thread * producer1_t = 
//         Thread::getInstance("producer thread 1", 0, 50);
//     Thread * consumer1_t = 
//         Thread::getInstance("consumer thread 1", 0, 150);
//     Thread * producer2_t = 
//         Thread::getInstance("producer thread 2",0, 100);
//     Thread * consumer2_t = 
//         Thread::getInstance("consumer thread 2", 0, 200);
    
//     consumer1_t->Fork(consumer,(void*)1);
//     producer1_t->Fork(producer,(void*)2);
//     producer2_t->Fork(producer,(void*)3);
//     consumer2_t->Fork(consumer,(void*)4);
// }

// class cond_buf {
// public:
//     int buf;
//     int n;
//     Lock *lock;
//     Condition *isFull;
//     Condition *isEmpty;
//     cond_buf(int n) {
//         this->n = n;
//         buf = 0;
//         lock = new Lock("condition lock");
//         isFull = new Condition("isFull Condition");
//         isEmpty = new Condition("isEmpty Condition");
//     }
//     ~cond_buf() {
//         delete lock;
//         delete isFull;
//         delete isEmpty;
//     }
//     void insert() {
//         lock->Acquire();
//         if (buf == n) {
//             printf("buf is full. %s is stuck here\n", 
//                 currentThread->getName());
//             this->isFull->Wait(lock);
//         }
//         buf++;
//         printf("%s did Operation insert, buf is %d\n",
//             currentThread->getName(), buf );
//         this->isEmpty->Signal(lock);
//         lock->Release();
//     }
//     void remove() {
//         lock->Acquire();
//         if (buf == 0) {
//             printf("buf is empty. %s is stuck here\n",
//                 currentThread->getName() );
//             this->isEmpty->Wait(lock);
//         }
//         buf--;
//         printf("%s did Operation remove, buf is %d\n",
//             currentThread->getName(), buf );
//         this->isFull->Signal(lock);
//         lock->Release();

//     }
// };
// cond_buf *cond_buf_P_C;
// void producer_cond (int which) {
//     while(currentThread->getTimeNeed() > 0) {
//         cond_buf_P_C->insert();
//         interrupt->OneTick();
        
//     }

//     DEBUG('t', "%s is finished\n", currentThread->getName());
// }
// void consumer_cond (int which) {
//     while(currentThread->getTimeNeed() > 0) {
//         cond_buf_P_C->remove();
//         interrupt->OneTick();
        
//     }
//     DEBUG('t', "%s is finished\n", currentThread->getName());
// }

// void ThreadTestP_C_cond() {
//     cond_buf_P_C = new cond_buf(10);
//     Thread * producer1_t = 
//         Thread::getInstance("producer thread 1", 0, 50);
//     Thread * consumer1_t = 
//         Thread::getInstance("consumer thread 1", 0, 150);
//     Thread * producer2_t = 
//         Thread::getInstance("producer thread 2",0, 100);
//     Thread * consumer2_t = 
//         Thread::getInstance("consumer thread 2", 0, 200);

//     consumer1_t->Fork(consumer_cond,(void*)1);
//     producer1_t->Fork(producer_cond,(void*)2);
//     producer2_t->Fork(producer_cond,(void*)3);
//     consumer2_t->Fork(consumer_cond,(void*)4);


// }

// Baraier *baraier;
// void testBaraier(int which) {

//     baraier->Baraier_pt(4);

//     printf("The end of %s\n", currentThread->getName());
// }


// void ThreadTestForBaraier() {
//     DEBUG('t',"Entering test func\n");
//     baraier = new Baraier("baraier");
//     Thread * thread[4];
    
//     thread[0] = 
//         Thread::getInstance("test thread 0",0, 50);
//     thread[1] = 
//         Thread::getInstance("test thread 1",0, 50);
//     thread[2] = 
//         Thread::getInstance("test thread 2",0, 50);
//     thread[3] = 
//         Thread::getInstance("test thread 3",0, 50);

//     thread[0]->Fork(testBaraier,(void*)0);
//     thread[1]->Fork(testBaraier,(void*)1);
//     thread[2]->Fork(testBaraier,(void*)2);
//     thread[3]->Fork(testBaraier,(void*)3);
// }
//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
    switch (testnum) {
    case 1:
//	    ThreadTestP_C();
	    break;
    case 2:
  //      ThreadTestP_C_cond();
        break;
    case 3:
 //       ThreadTestForBaraier();
        break;
    default:
	   printf("No test specified.\n");
	break;
    }
}

