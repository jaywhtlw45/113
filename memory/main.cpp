#include <iostream>
#include <bitset>
#include <vector>
#include <fstream>
#include <string>

using namespace std;

// Cache block structure
struct CacheBlock
{
    bool valid;         // false for invalid, true for valid
    bool coldStart = 1; // 0 for not cold start, 1 for cold start
    bool history;       // 0 MRU, 1 LRU
    bitset<4> tag;
    int data;
};

// Cache size, memory size, instruction bits, cache associativity
const int CACHE_SIZE = 16;
const int CACHE_ASSOC = 2;
const int I_BITS = 32;
const int MEM_SIZE = 128;

// Register file
int registers[8] = {0};

// Cache structure
vector<vector<CacheBlock>> cache(CACHE_SIZE / CACHE_ASSOC, vector<CacheBlock>(CACHE_ASSOC));

// Main memory
int memory[MEM_SIZE];

// Cache funcitons
void processInstruction(bitset<I_BITS> instruction);
void loadWord(bitset<5> rt, bitset<16> immediate);
void storeWord(bitset<5> rt, bitset<16> immediate);

// Helper functions
bitset<4> getTag(bitset<16> immediate);
int getIndex(bitset<16> immediate);
bitset<I_BITS> stringToBitset(string line);
void executeInstructions();

// Initialization and display functions
void initializeMemory();
void displayCache();
void displayMemory();
void displayRegisters();

int main()
{
    // Initialize memory, cache, and registers
    initializeMemory();
    executeInstructions();
    displayRegisters();
    displayCache();
    // displayMemory();

    return 0;
}

void swCacheHit(int index, int way, bitset<5> rt)
{
    // don't update history because this block cannot be used again

    // make dirty bit
    cache[index%8][way].valid = false;

    // write to cache
    cache[index%8][way].data = registers[rt.to_ulong() - 16];
}

void swCacheMiss(int index, bitset<5> rt, bitset<16> immediate)
{
    // write directly to memory
    memory[immediate.to_ulong()] = registers[rt.to_ulong() - 16];
}

void storeWord(bitset<5> rt, bitset<16> immediate)
{
    int index = getIndex(immediate);
    bitset<4> tag = getTag(immediate);

    bool cacheHit = false;
    int way = -1;

    // Check if the data is in the cache
    for (int i = 0; i < CACHE_ASSOC; ++i)
    {
        cout << "index: " << index << endl;
        cout << "cache[index%8][i].valid: " << cache[index%8][i].valid << endl;
        cout << "i: " << i << endl;
        cout << endl;
        if (cache[index%8][i].valid && cache[index%8][i].tag == tag)
        {
            cacheHit = true;
            way = i;
            break;
        }
    }

    if (cacheHit)
        swCacheHit(index, way, rt);
    else
        swCacheMiss(index, rt, immediate);
    // If it's a write operation, update the data in the cache and memory
    // if (write)
    // {
    //     int regIndex = (int_address >> 2) & 0x07; // 3 bits for register index
    //     cache[index][way].data = registers[regIndex];
    //     memory[int_address >> 2] = registers[regIndex];
    // }
    return;
}

void lwCacheHit(int index, int way, bitset<5> rt)
{
    // if hit update set history
    if (way == 0)
    {
        cache[index%8][0].history = 0;
        cache[index%8][1].history = 1;
    }
    // if hit update set history
    else if (way == 1)
    {
        cache[index%8][0].history = 1;
        cache[index%8][1].history = 0;
    }
    registers[rt.to_ulong() - 16] = cache[index%8][way].data;
}

void lwColdStart(int index, bitset<5> rt, bitset<16> immediate)
{
    cout << "cold start" << endl
         << endl;
    // if cold start do not write back to memory
    if (cache[index%8][0].coldStart == 1)
    {
        cache[index%8][0].coldStart = 0;
        cache[index%8][0].history = 0;
        cache[index%8][1].history = 1;

        cache[index%8][0].valid = true;
        cache[index%8][0].tag = getTag(immediate);
        cache[index%8][0].data = memory[immediate.to_ulong()];

        registers[rt.to_ulong() - 16] = cache[index%8][0].data;
    }
    // if cold start do not write back to memory
    else if (cache[index%8][1].coldStart == 1)
    {
        cache[index%8][1].coldStart = 0;
        cache[index%8][0].history = 1;
        cache[index%8][1].history = 0;

        cache[index%8][1].valid = 1;
        cache[index%8][1].tag = getTag(immediate);
        cache[index%8][1].data = memory[immediate.to_ulong()];

        registers[rt.to_ulong() - 16] = cache[index%8][1].data;
    }
}

void lwCacheMiss(int index, bitset<5> rt, bitset<16> immediate)
{
    // Find the LRU way
    int way = (cache[index%8][0].history && cache[index%8][1].history) ? 0 : 1;

    // Write back to memory
    memory[immediate.to_ulong()] = cache[index%8][way].data;

    // Update the cache block
    cache[index%8][way].valid = true;
    cache[index%8][way].tag = getTag(immediate);
    cache[index%8][way].data = memory[immediate.to_ulong()];

    // Update set history
    if (way == 0)
    {
        cache[index%8][0].history = 0;
        cache[index%8][1].history = 1;
    }
    else
    {
        cache[index%8][0].history = 1;
        cache[index%8][1].history = 0;
    }

    // Update Registers
    registers[rt.to_ulong() - 16] = cache[index%8][way].data;
}

void loadWord(bitset<5> rt, bitset<16> immediate)
{
    int index = getIndex(immediate);
    bitset<4> tag = getTag(immediate);

    bool cacheHit = false;
    int way = -1;

    // Check if the data is in the cache
    for (int i = 0; i < CACHE_ASSOC; ++i)
    {
        if (cache[index%8][i].valid && cache[index%8][i].tag == tag)
        {
            cacheHit = true;
            way = i;
            break;
        }
    }

    if (cacheHit)
        lwCacheHit(index, way, rt);
    else if (cache[index%8][0].coldStart == 1 || cache[index%8][1].coldStart == 1)
        lwColdStart(index, rt, immediate);
    else
        lwCacheMiss(index, rt, immediate);
}

// Function to read the instruction
// '100011' for load and '101011' for store
void processInstruction(bitset<I_BITS> instruction)
{
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

    if (opcode == 35)
        loadWord(rt, immediate);
    else if (opcode == 43)
        storeWord(rt, immediate);
    else
    {
        cout << "error" << endl;
        exit(1);
    }
}

int getIndex(bitset<16> immediate)
{
    bitset<3> index;
    for (int i = 0; i < 4; i++)
        index[i] = immediate[i];

    return index.to_ulong();
}

bitset<4> getTag(bitset<16> immediate)
{
    bitset<4> tag;
    for (int i = 3; i < 7; i++)
        tag[i - 3] = immediate[i];

    return tag;
}

void executeInstructions()
{
    ifstream inputFile("input_file.txt");

    if (!inputFile.is_open())
    {
        cerr << "Unable to open file" << endl;
    }

    bitset<I_BITS> instruction;
    string line;

    int counter = 1;
    while (getline(inputFile, line))
    {
        instruction = stringToBitset(line);
        processInstruction(instruction);
        // if (counter == 5)
        //     break;
        // counter++;
    }
    inputFile.close();
}

// Function to display main memory contents
void displayMemory()
{
    cout << "Memory Contents:" << endl;
    for (int i = 0; i < MEM_SIZE; ++i)
    {
        cout << "Block " << i << ": " << memory[i] << endl;
    }
    cout << endl;
}

// Function to initialize memory
void initializeMemory()
{
    for (int i = 0; i < MEM_SIZE; ++i)
    {
        memory[i] = i + 5;
    }
}

// Function to display cache contents
void displayCache()
{
    cout << "Cache Contents:" << endl;
    for (int i = 0; i < CACHE_SIZE / CACHE_ASSOC; ++i)
    {
        for (int j = 0; j < CACHE_ASSOC; ++j)
        {
            cout << "Set " << i << ", Way " << j << ": ";
            if (cache[i][j].valid)
            {
                cout << "CS:" << cache[i][j].coldStart << " H:" << cache[i][j].history;
                cout << " V:1 Tag:" << cache[i][j].tag << " Data:" << cache[i][j].data;
            }
            else
            {
                cout << "CS:" << cache[i][j].coldStart << " H:" << cache[i][j].history;
                cout << " V:0";
            }
            cout << endl;
        }
    }
    cout << endl;
}

bitset<I_BITS> stringToBitset(string line)
{
    bitset<I_BITS> instruction;
    for (int i = 0; i < I_BITS; i++)
    {
        if (line[i] == '1')
        {
            instruction.set(I_BITS - i - 1);
        }
    }
    return instruction;
}

void displayRegisters()
{
    for (int i = 0; i < 8; ++i)
    {
        cout << "R" << i << ": " << registers[i] << endl;
    }
    cout << endl;
}
