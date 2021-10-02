/*
Name: Elizabeth Park
Course: CSCE614-600
Date: 10/2/2021
*/
#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <Windows.h>
using namespace std;

char* getInstruction(char* buf) {
	//get the instruction
	char* startAddr = strchr(buf, ' ') + 1;
	char* endAddr = strstr(startAddr, "\r\n");
	int addrSize = endAddr - startAddr;
	char* addr = (char*)malloc(sizeof(int) * addrSize + 1);
	memcpy(addr, startAddr, addrSize);
	addr[addrSize] = '\0'; //null-terminate
	//string addrStr(addr);

	return addr;
}
char* readFile(string f) {
	wchar_t fname[20];
	mbstowcs(fname, f.c_str(), strlen(f.c_str()) + 1);
	LPWSTR filename = fname;
	HANDLE file = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE) { //catch CreateFile error
		printf("Error in %s: %d\n", __FUNCTION__, GetLastError());
		return 0;
	}
	// get file size
	LARGE_INTEGER li;
	BOOL bRet = GetFileSizeEx(file, &li);
	// read file into a buffer
	int fileSize = (DWORD)li.QuadPart;	// assumes file size is below 2GB; otherwise, an __int64 is needed
	DWORD bytesRead;
	char* fileBuf = new char[fileSize];
	bRet = ReadFile(file, fileBuf, fileSize, &bytesRead, NULL);
	if (bRet == 0 || bytesRead != fileSize) {
		printf("Error in %s: %d\n", __FUNCTION__, GetLastError());
		return 0;
	}
	fileBuf[fileSize] = '\0'; //null-terminate to use strstr, strchr
	return fileBuf;
}
bool extract(char* address, int* t, int* ind, int* off) {
	//tag, index, and offset are returned


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
	int* cache = new int[numSets * assoc]; //Cache 2d array
	for (int r = 0; r < numSets; r++) {
		for (int c = 0; c < assoc; c++) {
			cache[r * assoc + c] = -1; //initialize to -1
		}
	}

	//------------Read File--------------
	string filename = "example.txt"; //file name
	char* fileBuf = readFile(filename);
	//------------------Access Addresses-------------------
	// loop here later
	//get the instruction
	char inst = fileBuf[0]; //instruction (read or write)
	char* addr_hex = getInstruction(fileBuf);
	long long int addr = (int)strtol(addr_hex, NULL, 16);
	//extract tag, index, offset
	int addrSize = 64; //64 bit addr (fixed)
	int indLen = log2(numSets);
	int offLen = log2(blocksize);
	int tagLen = addrSize - indLen - offLen; //address is 64 bit
	int tag = addr >> (addrSize - tagLen); //shift bits
	int index = (addr >> offLen) - (tag << indLen); //a set index
	int offset = addr - ((addr >> offLen) << offLen); //a block index in a set
	//Check cache
	int cacheBlock = cache[index*assoc + offset];
	if (cacheBlock == -1) {
		inst == 'r' ? readMiss++ : writeMiss++;
	}
	return 0;
}