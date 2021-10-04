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
//for linux
#include <cmath> 
#include <algorithm>
#include <cstring>
using namespace std;

long long int getAddress(string li) {
	
	//get the address
	char* line_c = const_cast<char*>(li.c_str());
	char* startAddr = strchr(line_c, ' ') + 1;
	char* endAddr = strchr(startAddr, '\0');
	int addrSize = endAddr - startAddr;
	char* addr = (char*)malloc(sizeof(int) * addrSize + 1);
	memcpy(addr, startAddr, addrSize);
	addr[addrSize] = '\0'; //null-terminate

	long long int address = (long long int)strtoll(addr, NULL, 16); //convert hex to int
	return address;
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
	long int totalMiss = 0; long int totalAccess = 0;
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
		(inst == 'r') ? totalRead++ : totalWrite++;
		long long int addr = getAddress(line);
		//extract tag, index, offset
		long long int tag, index, offset;
		extract(addr, numSets, blocksize, &tag, &index, &offset);

		// Check cache
		if (assoc == 1) { // direct mapping
			if (cache[index] != tag) { // miss
				(inst == 'r') ? readMiss++ : writeMiss++;
				cache[index] = tag; // replace
			}
		}
		else { // fully or n-way set associativity

			// Check if the block is in the set or not
			long long int* nextSetblock = cache + index * assoc + assoc;
			long long int* existBlock = find(cache + index * assoc + 0, nextSetblock, tag);
			if (existBlock == nextSetblock) { // if miss
				// Increment miss
				(inst == 'r') ? readMiss++ : writeMiss++;
				// Replace block
				if (repl == "r") { // Random policy
					int set_index = rand() % assoc;
					cache[index * assoc + set_index] = tag;
				}
				else { // LRU policy
					// Find (most-recent) unused block
					int* nextSetbit = lru[index] + assoc;
					int* unused = find(lru[index], nextSetbit, 0); 
					if (unused != nextSetbit) { // found unused bit
						int dist = distance(lru[index], unused);
						cache[index * assoc + dist] = tag; // replace
						unused[0] = 1;
						last_offset[index] = dist;
					}
					else { // if all used
						// Reset used bits except most-recent
						fill(lru[index], lru[index] + assoc, 0);
						lru[index][last_offset[index]] = 1;
						// Replace
						cache[index * assoc + last_offset[index]] = tag;
					}
				}
			}
			else { //hit
				//do nothing
			}

		}

	}
	
	totalMiss = readMiss + writeMiss;
	totalAccess = totalRead + totalWrite;
	printf("%ld %f%%\t%d %f%%\t%d %f%%\n", totalMiss, (double(totalMiss) / totalAccess) * 100, readMiss, 100 * (double(readMiss) / totalRead), writeMiss, 100 * (double(writeMiss) / totalWrite));
	return 0;
}