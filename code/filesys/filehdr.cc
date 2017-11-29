// filehdr.cc 
//	Routines for managing the disk file header (in UNIX, this
//	would be called the i-node).
//
//	The file header is used to locate where on disk the 
//	file's data is stored.  We implement this as a fixed size
//	table of pointers -- each entry in the table points to the 
//	disk sector containing that portion of the file data
//	(in other words, there are no indirect or doubly indirect 
//	blocks). The table size is chosen so that the file header
//	will be just big enough to fit in one disk sector, 
//
//      Unlike in a real system, we do not keep track of file permissions, 
//	ownership, last modification date, etc., in the file header. 
//
//	A file header can be initialized in two ways:
//	   for a new file, by modifying the in-memory data structure
//	     to point to the newly allocated data blocks
//	   for a file already on disk, by reading the file header from disk
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "filehdr.h"

#include <ctime>
#include <cstring>

//----------------------------------------------------------------------
// FileHeader::Allocate
// 	Initialize a fresh file header for a newly created file.
//	Allocate data blocks for the file out of the map of free disk blocks.
//	Return FALSE if there are not enough free blocks to accomodate
//	the new file.
//
//	"freeMap" is the bit map of free disk sectors
//	"fileSize" is the bit map of free disk sectors
//----------------------------------------------------------------------

bool
FileHeader::Allocate(BitMap *freeMap, int fileSize)
{ 
	printf("Allocate file size %d\n",fileSize );
    numBytes = fileSize;
    numSectors  = divRoundUp(fileSize, SectorSize);

    // use primary directory can hold the sectors
    if (numSectors <= NumFirstDirect) {
    	if (freeMap->NumClear() < numSectors){
    		printf("No enough space\n");
	    	return FALSE;		// not enough space
    	}
		printf("FileHeader::Allocate numSectors %d\n", numSectors);
	    for (int i = 0; i < numSectors; i++) {
		    dataSectors[i] = freeMap->Find();
		    printf("sector %d\n", dataSectors[i]);
	    }
	    return TRUE;	

    } 
    // need secondary directory
    else {
    	int secondSectorNeed = divRoundUp(fileSize - NumFirstDirect*SectorSize,
    									 SectorSize);
    	int secondDirectNeed = divRoundUp(secondSectorNeed, 
    								SectorSize/sizeof(int));

    	if (freeMap->NumClear() < NumFirstDirect + secondSectorNeed + 
    			secondDirectNeed) {
    		printf("not enough space\n");
    		return FALSE;
    	}

    	for (int i = 0;i < NumFirstDirect;i++) {
    		printf("allocated first level index\n");
    		dataSectors[i] = freeMap->Find();
    	}
    	int sectorNeedToAlloc = secondSectorNeed;
    	int sectorArray[SectorSize/(sizeof(int))];
    	for (int i = NumFirstDirect; i < NumFirstDirect + secondDirectNeed;
    			++i) {
    		printf("allocated second level directory\n");
    		dataSectors[i] = freeMap->Find();
    		memset(sectorArray, -1, sizeof(sectorArray));
    		for (int k = 0;k < SectorSize/sizeof(int); ++k) {
    			printf("allocated second level index\n");
    			sectorArray[k] = freeMap->Find();
    			sectorNeedToAlloc--;
    			if (sectorNeedToAlloc == 0) {
    				break;
    			}
    		}
    		synchDisk->WriteSector(dataSectors[i], (char*)sectorArray);


    	}




    }
    
}
//-----------------------------------------------------------------------
// FileHeader::ReAllocate
//  ReAllocate more bytes to the file
//  
//  return TRUE for success, FALSE for fail. 
//-----------------------------------------------------------------------
bool  
FileHeader::ReAllocate(BitMap *freeMap, int ReAllocateSize) {
	int newSize = numBytes + ReAllocateSize;
	int totalSectors = divRoundUp(newSize, SectorSize);

	if (totalSectors == numSectors) {
		printf("Dont need to allocated new sector\n");
		numBytes = newSize;

		return TRUE;
	}
	// else need allocate new sectors

	if (totalSectors <= NumFirstDirect) {
		if (freeMap->NumClear() < totalSectors - numSectors) {
			printf("No enough space\n");
			return FALSE;
		}
		printf("totalSectors <= NumFirstDirect\n");
		printf("FileHeader::ReAllocate %d Sectors\n",totalSectors - numSectors);
		for (int i = numSectors; i < totalSectors;++i) {
			dataSectors[i] = freeMap->Find();
			printf("Sector %d\n", dataSectors[i]);
		}

		numBytes = newSize;
		numSectors = totalSectors;
		return TRUE;
	}

	else if (numSectors <= NumFirstDirect && totalSectors > NumFirstDirect) {
		printf("numSectors <= NumFirstDirect  && totalSectors > NumFirstDirect\n");
		int extraSector = totalSectors - NumFirstDirect;
		int extraIndexSector = divRoundUp(extraSector, SectorSize/sizeof(int));

		for (int i = numSectors; i < NumFirstDirect;++i) {
			dataSectors[i] = freeMap->Find();
		}

		int sectorArray[SectorSize/sizeof(int)];
		int sectorNeedToAlloc = extraSector;

		for (int i = NumFirstDirect; i < NumFirstDirect + extraIndexSector;
							++i) {
			dataSectors[i] = freeMap->Find();
    		memset(sectorArray, -1, sizeof(sectorArray));

    		for (int k = 0;k < SectorSize/sizeof(int);k++) {
    			sectorArray[i] = freeMap->Find();
    			sectorNeedToAlloc--;
    			if(sectorNeedToAlloc == 0) {
    				break;
    			}
    		}

    		synchDisk->WriteSector(dataSectors[i], (char*)sectorArray);

		}
		numBytes = newSize;
		numSectors = totalSectors;
		return TRUE;
	} else {
		printf("numSectors > NumFirstDirect\n");
		int extraSector = totalSectors - numSectors;
		int originEndIndexSector = (numSectors - NumFirstDirect)/ 
						(SectorSize/sizeof(int)) + NumFirstDirect ;

		int extraNumSectors = (totalSectors - NumFirstDirect)/
					(SectorSize/sizeof(int)) + NumFirstDirect 
								- originEndIndexSector; 

		int sectorArray[SectorSize/sizeof(int)];
		synchDisk->ReadSector(dataSectors[originEndIndexSector], 
					(char*)sectorArray);

		for (int i = 0; i < SectorSize/(sizeof(int)); ++i) {
			if (sectorArray[i] == -1) {
				sectorArray[i] = freeMap->Find();
				extraSector--;
				if (extraSector == 0) 
					break;
			}
		}
		synchDisk->WriteSector(dataSectors[originEndIndexSector],
				(char*)sectorArray);
		if (extraSector == 0) {
			numBytes = newSize;
			numSectors = totalSectors;
			return TRUE;
		}
		for (int i = 0;i < extraNumSectors;++i) {
			dataSectors[i] = freeMap->Find();
			memset(sectorArray, -1, sizeof(sectorArray));

			for (int k = 0; k < SectorSize/sizeof(int);++k) {
				sectorArray[k] = freeMap->Find();
				extraSector--;
				if(extraSector == 0)
					break;
			}
			synchDisk->WriteSector(dataSectors[i], (char*)sectorArray);
		}
		numBytes = newSize;
		numSectors = totalSectors;
		return TRUE;

	}

}

//----------------------------------------------------------------------
// FileHeader::Deallocate
// 	De-allocate all the space allocated for data blocks for this file.
//
//	"freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------

void 
FileHeader::Deallocate(BitMap *freeMap)
{
	if (numSectors <= NumFirstDirect) {
	    for (int i = 0; i < numSectors; i++) {
			ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
			freeMap->Clear((int) dataSectors[i], FALSE);
	    }
	}
	else {
		int secondSectorNeed = divRoundUp(numBytes - NumFirstDirect*SectorSize,
    									 SectorSize);
    	int secondDirectNeed = divRoundUp(secondSectorNeed, 
    								SectorSize/sizeof(int));

    	for (int i = 0;i < NumFirstDirect;i++) {
    		ASSERT(freeMap->Test((int)dataSectors[i]));
    		freeMap->Clear((int) dataSectors[i], FALSE);
    	}

    	int sectorNeedToDeAlloc = secondSectorNeed;
    	int sectorArray[SectorSize/(sizeof(int))];

    	for (int i = NumFirstDirect; i < NumFirstDirect + secondDirectNeed;
    				++i) {
    		synchDisk->ReadSector(dataSectors[i], (char*)sectorArray);
    		for (int k = 0;k < SectorSize/sizeof(int);++k) {
    			ASSERT(freeMap->Test((int)sectorArray[k]));
    			freeMap->Clear((int)sectorArray[i], FALSE);
    			sectorNeedToDeAlloc--;
    			if (sectorNeedToDeAlloc == 0) {
    				break;
    			}
    		}

    		ASSERT(freeMap->Test((int)dataSectors[i]));
    		freeMap->Clear((int)dataSectors[i], FALSE);


    	}


	}
}

//----------------------------------------------------------------------
// FileHeader::FetchFrom
// 	Fetch contents of file header from disk. 
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void
FileHeader::FetchFrom(int sector)
{
    synchDisk->ReadSector(sector, (char *)this);
}

//----------------------------------------------------------------------
// FileHeader::WriteBack
// 	Write the modified contents of the file header back to disk. 
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void
FileHeader::WriteBack(int sector)
{
    synchDisk->WriteSector(sector, (char *)this); 
}

//----------------------------------------------------------------------
// FileHeader::ByteToSector
// 	Return which disk sector is storing a particular byte within the file.
//      This is essentially a translation from a virtual address (the
//	offset in the file) to a physical address (the sector where the
//	data at the offset is stored).
//
//	"offset" is the location within the file of the byte in question
//----------------------------------------------------------------------

int
FileHeader::ByteToSector(int offset)
{
    // return (dataSectors[offset / SectorSize]);

    if (offset <= NumFirstDirect*SectorSize) {
    	return (dataSectors[offset / SectorSize]);
    } else {
    	int secondIndexNum = (offset - NumFirstDirect*SectorSize) / 
    		(SectorSize * (SectorSize / sizeof(int)));
    	int sectorArray[(SectorSize)/sizeof(int)];
    	synchDisk->ReadSector(dataSectors[secondIndexNum + NumFirstDirect], 
    				(char*)sectorArray);
    	return sectorArray[(offset - NumFirstDirect*SectorSize - secondIndexNum*
    		SectorSize*(SectorSize/sizeof(int)) )/SectorSize]; 
    }
}

//----------------------------------------------------------------------
// FileHeader::FileLength
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

int
FileHeader::FileLength()
{
    return numBytes;
}

//----------------------------------------------------------------------
// FileHeader::Print
// 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
//----------------------------------------------------------------------

void
FileHeader::Print()
{
    int i, j, k;
    char *data = new char[SectorSize];

    printf("FileHeader contents.  File size: %d.  File blocks:\n", numBytes);
    for (i = 0; i < numSectors; i++)
	printf("%d ", dataSectors[i]);
    printf("\nFile contents:\n");
    for (i = k = 0; i < numSectors; i++) {
	    synchDisk->ReadSector(dataSectors[i], data);
        for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
	        if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
		        printf("%c", data[j]);
            else
		        printf("\\%x", (unsigned char)data[j]);
	    }
        printf("\n"); 
    }
    delete [] data;
}


void 
FileHeader::setCreateTime() {
    time_t t;

    t = time(NULL);

    strcpy(this->createTime, ctime(&t));

}


char * 
FileHeader::getCreateTime() {
    return this->createTime;
}

void 
FileHeader::setLastVisitTime() {
    time_t t;

    t = time(NULL);

    strcpy(this->lastVisitTime, ctime(&t));

}

char *
FileHeader::getLastVisitTime() {
    return this->lastVisitTime;
}

void 
FileHeader::setLastChangeTime() {
    time_t t;

    t = time(NULL);

    strcpy(this->lastChangeTime, ctime(&t));
}

char * 
FileHeader::getLastChangeTime() {
    return this->lastChangeTime;
}