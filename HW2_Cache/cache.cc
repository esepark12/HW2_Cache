/*
Name: Elizabeth Park
Course: CSCE614-600
Date: 10/2/2021
*/
#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <fstream>
using namespace std;

char* getAddress(char* li) {
	
	//get the address
	char* startAddr = strchr(li, ' ') + 1;
	char* endAddr = strchr(startAddr, '\0');
	int addrSize = endAddr - startAddr;
	char* addr = (char*)malloc(sizeof(int) * addrSize + 1);
	memcpy(addr, startAddr, addrSize);
	addr[addrSize] = '\0'; //null-terminate

	return addr;
}
bool extract(long long int addr, int numsets, int bsize, long long int *t, long long int* ind, long long int* off) {
	//tag, index, and offset are returned
	int addrSize = 64; //64 bit addr (fixed)
	int indLen = log2(numsets);
	int offLen = log2(bsize);
	int tagLen = addrSize - indLen - offLen; //address is 64 bit
	*t = addr >> (addrSize - tagLen); //shift bits
	*ind = (addr >> offLen) - (*t << indLen); //a set index
	*off = addr - ((addr >> offLen) << offLen); //a block index in a set

	return 0;
}
int main(int argc, char* argv[]) {
	/*
	int nk = atoi(argv[1]); //the capacity of the cache in kilobytes
	int assoc = atoi(argv[2]); //the associativity of the cache
	int blocksize = atoi(argv[3]); //the size of a single cache block in bytes
	char* repl = argv[4]; //the replacement policy
	*/
	//------------Initialize cache variables----------
	int nk = 2048; //the capacity of the cache in kilobytes
	int assoc = 64; //the associativity of the cache (number of blocks in a set)
	int blocksize = 64; //the size of a single cache block in bytes
	string repl = "l"; //the replacement policy
	int numSets = nk * pow(2, 10) / (assoc * blocksize);
	int totalMiss = 0; int totalAccess = 0;
	int readMiss = 0; int totalRead = 0;
	int writeMiss = 0; int totalWrite = 0;
	long long int* cache = new long long int[numSets * assoc]; //Cache 2d array
	memset(cache, -1, sizeof(long long int) * numSets * assoc); //initialize to -1
	//int* lru = new int[numSets * assoc]; //keeps track of used/unused blocks
	//memset(lru, 0, sizeof(int) * numSets * assoc); //initialize to 0
	int** lru = new int* [numSets];
	for (int i = 0; i < numSets; ++i) //initialize to 0
		lru[i] = new int[assoc];
	for (int i = 0; i < numSets; ++i) {
		for (int j = 0; j < assoc; ++j) {
			lru[i][j] = 0;
		}
	}
	//------------Get File--------------
	string filename = "429.mcf-184B.trace.txt"; //file name
	ifstream file(filename);
	//------------------Access Addresses-------------------
	int* last_offset = new int[assoc]; //used to check recently-used block in each set
	memset(last_offset, -1, sizeof(int) * assoc);

	string line;
	while (getline(file, line)) {
		//get the instruction
		char inst = line[0]; //instruction (read or write)
		char* line_c = const_cast<char*>(line.c_str());
		char* addr_hex = getAddress(line_c);
		long long int addr = (long long int)strtoll (addr_hex, NULL, 16); //convert hex to int
		(inst == 'r') ? totalRead++ : totalWrite++;
		//extract tag, index, offset
		long long int tag, index, offset;
		extract(addr, numSets, blocksize, &tag, &index, &offset);

		//Check cache using LRU

		//First check if the block is contained in the set or not
		long long int* nextSetblock = cache + index * assoc + assoc;
		long long int* existBlock = find(cache + index*assoc+ 0, nextSetblock, tag);
		if (existBlock == nextSetblock) { //if miss: identical block not found
			//increment miss
			(inst == 'r') ? readMiss++ : writeMiss++;
			//Allocate/replace block into cache
			int* nextSetbit = lru[index] + assoc;
			int* unused = find(lru[index], nextSetbit, 0); //find (recently)unused block
			if (unused != nextSetbit) { //found unused bit
				int dist = distance(lru[index], unused);
				cache[index * assoc + dist] = tag; //replace
				unused[0] = 1;
				last_offset[index] = dist;
			}
			else { //if all used
				//reset used bits except most recent
				fill(lru[index], lru[index] + assoc, 0);
				lru[index][last_offset[index]] = 1;
				//replace
				cache[index * assoc + last_offset[index]] = tag;
			}

		}
		else { //no miss
			//do nothing
			//printf("hello\n");
		}

	}

	totalMiss = readMiss + writeMiss;
	totalAccess = totalRead + totalWrite;
	printf("%d %f%%\t%d %f%%\t%d %f%%", totalMiss, (double(totalMiss) / totalAccess) * 100, readMiss, 100 * (double(readMiss) / totalRead), writeMiss, 100 * (double(writeMiss) / totalWrite));
	return 0;
}