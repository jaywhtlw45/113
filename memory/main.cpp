// Jason Whitlow
// CSCI 113
// Program 8
// This program simulates a cache memory system with a 2-way set associative cache.

//! NOTE: This program assumes that the the rs bits are zero for simplicity.

#include <iostream>
#include <bitset>
#include <vector>
#include <fstream>
#include <string>

using namespace std;

// cache block structure
struct CacheBlock
{
    bool valid = 0; // 0 for cold start, 1 for valid
    bool history;   // 1 MRU, 0 LRU
    bitset<4> tag;
    int data;
};

// cache size, memory size, instruction bits, cache associativity
const int CACHE_SIZE = 16;
const int CACHE_ASSOC = 2;
const int MEM_SIZE = 128;

// register file
int registers[8] = {0};

// cache structure
vector<vector<CacheBlock>> cache(CACHE_SIZE / CACHE_ASSOC, vector<CacheBlock>(CACHE_ASSOC));

// main memory
int memory[MEM_SIZE];

// fetch and decode instructions
void fetchInstructions();
void decodeInstruction(bitset<32> instruction);

// execute load word instruction
void execLoadWord(bitset<5> rt, bitset<16> immediate);
void lwMiss(int index, bitset<5> rt, bitset<16> immediate);

// execute store word instruction
void execStoreWord(bitset<5> rt, bitset<16> immediate);

// find least recently used block, and write back address
int findLRUBlock(int index);
int findWBAddress(int index, int block);
void updateHistoryBits(int index, int block);

// helper functions
int getAddress(bitset<16> immediate);
int getIndex(bitset<16> immediate);
bitset<4> getTag(bitset<16> immediate);
bitset<32> stringToBitset(string line);

// initialization and display functions
void initializeMemory();
void initializeRegisters();
void displayCache();
void displayMemory();
void displayRegisters();
bool PRINT_ZEROES = 1;

int main()
{
    initializeMemory();
    initializeRegisters();
    cout << endl;

    // fetch, decode, then execute instructions
    fetchInstructions();

    // display registers, cache, and memory
    displayRegisters();
    displayCache();
    displayMemory();

    return 0;
}

// execute store word instruction
void execStoreWord(bitset<5> rt, bitset<16> immediate)
{
    int index = getIndex(immediate);
    bitset<4> tag = getTag(immediate);

    bool cacheHit = false;
    int block = -1;

    // Check if the data is in the cache
    for (int i = 0; i < CACHE_ASSOC; ++i)
    {
        if (cache[index][i].valid && cache[index][i].tag == tag)
        {
            cacheHit = true;
            block = i;
            break;
        }
    }

    if (cacheHit)
    {
        // update history bits, and write to cache
        cout << "sw hit" << endl;
        updateHistoryBits(index, block);
        cache[index][block].data = registers[rt.to_ulong() - 16];
    }
    else
    {
        // write directly to memory
        cout << "sw miss" << endl;
        int address = getAddress(immediate);
        memory[address] = registers[(rt.to_ulong() - 16)];
    }
    return;
}

// execute load word instruction
void execLoadWord(bitset<5> rt, bitset<16> immediate)
{
    int index = getIndex(immediate);
    bitset<4> tag = getTag(immediate);

    bool cacheHit = false;
    int block = -1;

    // Check if the data is in the cache
    for (int i = 0; i < CACHE_ASSOC; ++i)
    {
        if (cache[index][i].valid && cache[index][i].tag == tag)
        {
            cacheHit = true;
            block = i;
            break;
        }
    }

    if (cacheHit)
    {
        // update history bits, and write to register
        cout << "lw hit" << endl;
        updateHistoryBits(index, block);
        registers[rt.to_ulong() - 16] = cache[index][block].data;
    }
    else
    {
        cout << "lw miss" << endl;
        lwMiss(index, rt, immediate);
    }
}

// read miss
void lwMiss(int index, bitset<5> rt, bitset<16> immediate)
{
    // select the victim block, and set history bits
    int block = findLRUBlock(index);
    updateHistoryBits(index, block);

    // if the block is valid, write back to memory
    if (cache[index][block].valid == 1)
    {
        int writeBackAddress = findWBAddress(index, block);
        memory[writeBackAddress] = cache[index][block].data;
    }

    // set cache data, tag, and valid bit
    int address = getAddress(immediate);
    cache[index][block].data = memory[address];
    cache[index][block].tag = getTag(immediate);
    cache[index][block].valid = true;

    // read from cache to register
    registers[rt.to_ulong() - 16] = cache[index][block].data;
}

// read instrctions from file
void fetchInstructions()
{
    ifstream inputFile("input_file.txt");

    if (!inputFile.is_open())
    {
        cerr << "Unable to open file" << endl;
    }

    bitset<32> instruction;
    string line;

    // read instruction from file
    int count = 0;
    while (getline(inputFile, line))
    {
        instruction = stringToBitset(line);
        cout << instruction << " \t";
        decodeInstruction(instruction);
    }
    cout << endl;
    inputFile.close();
}

// decode the instruction and then execute store word or load word
void decodeInstruction(bitset<32> instruction)
{
    // NOTE: rs (bits 21-25) is assumed to be zero for simplicity.
    bitset<6> opcode; // bits 26-31
    for (int i = 0; i < 6; i++)
    {
        opcode[i] = instruction[i + 26];
    }
    bitset<5> rt; // bits 16-20
    for (int i = 0; i < 5; i++)
    {
        rt[i] = instruction[i + 16];
    }
    bitset<16> immediate;
    for (int i = 0; i < 16; i++)
    {
        immediate[i] = instruction[i];
    }

    // '100011' for load and '101011' for store
    if (opcode == 35)
        execLoadWord(rt, immediate);
    else if (opcode == 43)
        execStoreWord(rt, immediate);
    else
    {
        cout << "error" << endl;
        exit(1);
    }
}

// return index from immediate value, bits 2-5
int getIndex(bitset<16> immediate)
{
    bitset<4> index("000");
    for (int i = 2; i < 6; i++)
        index[i - 2] = immediate[i];
    index[3] = 0;

    return index.to_ulong();
}

// return tag from immediate value, bits 6-9
bitset<4> getTag(bitset<16> immediate)
{
    bitset<4> tag;
    for (int i = 5; i < 9; i++)
        tag[i - 5] = immediate[i];
    return tag;
}

// converts the immediate byte address to a word address
int getAddress(bitset<16> immediate)
{
    bitset<14> address;
    for (int i = 2; i < 16; i++)
        address[i - 2] = immediate[i];
    return address.to_ulong();
}

void displayMemory()
{
    cout << "Addr\tData" << endl;
    for (int i = 0; i < MEM_SIZE; ++i)
    {
        bitset<32> mem(memory[i]);
        cout << i << ":\t" << mem << endl;
    }
    cout << endl;
}

void initializeMemory()
{
    for (int i = 0; i < MEM_SIZE; ++i)
        memory[i] = i + 5;
}

void displayCache()
{
    for (int i = 0; i < CACHE_ASSOC; i++)
    {
        cout << "Cache Block " << i << endl;
        cout << "Set#\tValid\tHist\tTag\tData" << endl;
        for (int j = 0; j < CACHE_SIZE / CACHE_ASSOC; ++j)
        {
            bitset<32> data(cache[j][i].data);

            cout << j << "\t";
            if (cache[j][i].valid)
            {
                cout << "1\t";
                cout << cache[j][i].history << "\t";
                cout << cache[j][i].tag << "\t";
                cout << data << endl;
            }
            else
            {
                // print zeroes instead of empty
                if (PRINT_ZEROES)
                {

                    bitset<32> empty(0);
                    cout << "0\t";

                    if (cache[j][i].history)
                        cout << "1\t";
                    else
                        cout << "0\t";

                    cout << "0000\t";
                    cout << empty << endl;
                }
                else
                {
                    cout << "0\t";
                    if (cache[j][i].history)
                        cout << "1\t";
                    else
                        cout << "0\t";
                    cout << endl;
                }
            }
        }
        cout << endl;
    }
}

bitset<32> stringToBitset(string line)
{
    bitset<32> instruction;
    for (int i = 0; i < 32; i++)
    {
        if (line[i] == '1')
        {
            instruction.set(32 - i - 1);
        }
    }
    return instruction;
}

void displayRegisters()
{
    cout << "Registers" << endl;
    for (int i = 0; i < 8; ++i)
    {
        bitset<32> reg(registers[i]);
        cout << "$s" << i << ": " << reg << endl;
    }
    cout << endl;
}

void initializeRegisters()
{
    for (int i = 0; i < 8; ++i)
    {
        registers[i] = 0;
    }
}

// find least recently used block
int findLRUBlock(int index)
{
    int block = -1;

    // cold start, choose block 0
    if (cache[index][0].valid == 0 && cache[index][1].valid == 0)
    {
        block = 0;
    }
    // choose block 0
    else if (cache[index][0].history == 0)
    {
        block = 0;
    }
    // choose block 1
    else if (cache[index][1].history == 0)
    {
        block = 1;
    }
    return block;
}

void updateHistoryBits(int index, int block)
{
    if (block == 0)
    {
        cache[index][0].history = 1;
        cache[index][1].history = 0;
    }
    else if (block == 1)
    {
        cache[index][0].history = 0;
        cache[index][1].history = 1;
    }
}

// find write back address
int findWBAddress(int index, int block)
{
    bitset<7> address;
    address = index;

    for (int i = 3; i < 7; i++)
    {
        address[i] = 0;
        address[i] = cache[index][block].tag[i - 3];
    }
    return address.to_ulong();
}