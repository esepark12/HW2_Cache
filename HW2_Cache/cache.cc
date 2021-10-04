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
	/*
	* Given an instruction line
	* Returns address in type long long int
	*/

	char* line_c = const_cast<char*>(li.c_str());
	char* startAddr = strchr(line_c, ' ') + 1;
	char* endAddr = strchr(startAddr, '\0');
	int addrSize = endAddr - startAddr;
	char* addr = (char*)malloc(sizeof(int) * addrSize + 1);
	memcpy(addr, startAddr, addrSize);
	addr[addrSize] = '\0'; // null-terminate
	long long int address = (long long int)strtoll(addr, NULL, 16); // convert hex to int

	return address;
}
bool extract(long long int addr, int numsets, int bsize, long long int *t, long long int* ind, long long int* off) {
	/*
	* Given an address, num of sets, block size
	* Returns tag, index, offset of a block
	*/

	int addrSize = 64; //64 bit addr (fixed)
	int indLen = log2(numsets);
	int offLen = log2(bsize);
	int tagLen = addrSize - indLen - offLen;
	*t = addr >> (addrSize - tagLen); //addr in mem
	*ind = (addr >> offLen) - (*t << indLen); //a set index
	*off = addr - ((addr >> offLen) << offLen); //a block index in a set

	return 0;
}
int main(int argc, char* argv[]) {
	
	int nk = atoi(argv[1]); // the capacity of the cache in kilobytes
	int assoc = atoi(argv[2]); // the associativity of the cache
	int blocksize = atoi(argv[3]); // the size of a single cache block in bytes
	char repl = argv[4][0]; // the replacement policy
	
	//------------Initialize cache variables----------
	long int totalMiss = 0; long int totalAccess = 0;
	int readMiss = 0; int totalRead = 0;
	int writeMiss = 0; int totalWrite = 0;
	int numSets = nk * pow(2, 10) / (assoc * blocksize);
	long long int* cache = new long long int[numSets * assoc]; // Cache 2d array
	memset(cache, -1, sizeof(long long int) * numSets * assoc); // initialize to -1
	long long int* valid = new long long int[numSets * assoc]; // keeps track of valid and invalid blocks
	memset(cache, 0, sizeof(long long int) * numSets * assoc);
	int** lru = new int* [numSets]; // keeps track of frequently used/unused blocks in a cache
	for (int i = 0; i < numSets; ++i) 
		lru[i] = new int[assoc];
	for (int i = 0; i < numSets; ++i) { // initialize to 0
		for (int j = 0; j < assoc; ++j) {
			lru[i][j] = 0;
		}
	}

	//------------Get File--------------
	string filename = "429.mcf-184B.trace.txt";
	ifstream file(filename);
	//------------------Access Addresses-------------------
	int* last_offset = new int[assoc]; // used to check most-recent used block in each set
	memset(last_offset, -1, sizeof(int) * assoc);

	string line;
	while (getline(file, line)) {
		// Get the instruction
		char inst = line[0]; // instruction (read or write)
		(inst == 'r') ? totalRead++ : totalWrite++;
		long long int addr = getAddress(line);
		// Get tag, index, offset
		long long int tag, index, offset;
		extract(addr, numSets, blocksize, &tag, &index, &offset);

		// Check if block is in the cache
		if (assoc == 1) { // Direct mapping
			if (valid[index] == 0) { // invalid
				cache[index] = tag; // replace
				valid[index] = 1;
			}
			else if (cache[index] != tag) { // miss
				(inst == 'r') ? readMiss++ : writeMiss++;
				cache[index] = tag; // replace
			}
		}
		else { // Fully or n-way set associativity
			// First check for any invalid block
			long long int* nextSetvalid = valid + index * assoc + assoc;
			long long int* existInvalid = find(valid + index * assoc + 0, nextSetvalid, 0);
			if (existInvalid != nextSetvalid) { // invalid exists
				int dist = distance(valid + index * assoc + 0, existInvalid);
				cache[index * assoc + dist] = tag; // replace
				existInvalid[0] = 1; // update valid
			}
			else { // if valid
				// Check hit or miss
				long long int* nextSetblock = cache + index * assoc + assoc;
				long long int* existBlock = find(cache + index * assoc + 0, nextSetblock, tag);
				if (existBlock == nextSetblock) { // if miss (tag don't match)
					// Increment miss
					(inst == 'r') ? readMiss++ : writeMiss++;
					// Replace block
					if (repl == 'r') { // use Random policy
						int set_index = rand() % assoc;
						cache[index * assoc + set_index] = tag; // replace
					}
					else { // use LRU policy
						// Find (most-recent) unused block & replace
						int* nextSetbit = lru[index] + assoc;
						int* unused = find(lru[index], nextSetbit, 0);
						if (unused != nextSetbit) { // found unused bit
							int dist = distance(lru[index], unused);
							cache[index * assoc + dist] = tag; // replace
							unused[0] = 1; // update lru
							last_offset[index] = dist;
						}
						else { // if all used
							// Reset used bits except most-recent
							fill(lru[index], lru[index] + assoc, 0);
							cache[index * assoc + last_offset[index]] = tag; // replace
							lru[index][last_offset[index]] = 1;
						}
					}
				}
				else { // hit
					// do nothing
				}
			}
		}

	}
	
	totalMiss = readMiss + writeMiss;
	totalAccess = totalRead + totalWrite;
	printf("%ld %f%%\t%d %f%%\t%d %f%%\n", totalMiss, (double(totalMiss) / totalAccess) * 100, readMiss, 100 * (double(readMiss) / totalRead), writeMiss, 100 * (double(writeMiss) / totalWrite));
	
	return 0;
}