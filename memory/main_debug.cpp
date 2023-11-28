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
    bool history;   // 0 MRU, 1 LRU
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

// execute load word instruction, cache hit, and cache miss
void execLoadWord(bitset<5> rt, bitset<16> immediate);
void lwHit(int index, int block, bitset<5> rt);
void lwMiss(int index, bitset<5> rt, bitset<16> immediate);

// execute store word instruction, cache hit, and cache miss
void execStoreWord(bitset<5> rt, bitset<16> immediate);
void swHit(int index, int block, bitset<5> rt);
void swMiss(int index, bitset<5> rt, bitset<16> immediate);

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
void displayCache();
void displayMemory();
void displayRegisters();

int main()
{
    initializeMemory();

    // fetch, decode, then execute instructions
    fetchInstructions();

    // displayRegisters();
    displayCache();
    // displayMemory();

    return 0;
}

// store word
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

    cout << "rt " << rt << endl;
    cout << "index: " << index << endl;
    cout << "tag: " << tag << endl;

    if (cacheHit)
    {
        cout << "cache hit" << endl;
        cout << "store word from reg " << rt.to_ullong() - 16 << " to index " << index << endl;
        swHit(index, block, rt);
    }
    else
    {
        cout << "cache miss" << endl;
        cout << "store word from reg " << rt.to_ullong() - 16 << " to index " << index << endl;
        swMiss(index, rt, immediate);
    }
    return;
}

// store word, write hit
void swHit(int index, int block, bitset<5> rt)
{
    // update history bits, and write to cache
    updateHistoryBits(index, block);
    cache[index][block].data = registers[rt.to_ulong() - 16];
}

// store word, write miss
void swMiss(int index, bitset<5> rt, bitset<16> immediate)
{
    // write directly to memory
    int address = getAddress(immediate);
    memory[address] = registers[(rt.to_ulong() - 16)];
}

void execLoadWord(bitset<5> rt, bitset<16> immediate)
{
    int index = getIndex(immediate);
    bitset<4> tag = getTag(immediate);

    cout << "index: " << index << endl;
    cout << "tag: " << tag << endl;

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

    // displayMemory();

    if (cacheHit)
    {
        cout << "cache hit, load word" << endl;
        lwHit(index, block, rt);
    }
    else
    {
        cout << "cache miss, load word" << endl;
        lwMiss(index, rt, immediate);
    }
}

// read hit
void lwHit(int index, int block, bitset<5> rt)
{
    // update history bits, and write to register
    updateHistoryBits(index, block);
    registers[rt.to_ulong() - 16] = cache[index][block].data;
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
        if (count == 11)
        {
            memory[11] = 1;
        }
        instruction = stringToBitset(line);
        cout << instruction;
        decodeInstruction(instruction);
        if (count > 13)
        {
            break;
        }
        if (count > 6)
        {
            displayRegisters();
            displayCache();
            displayMemory();
        }

        count++;
        cout << endl;
    }
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

int getAddress(bitset<16> immediate)
{
    bitset<14> address;
    for (int i = 2; i < 16; i++)
        address[i - 2] = immediate[i];

    cout << "address.toUlong: " << address.to_ulong() << endl;
    return address.to_ulong();
}

void displayMemory()
{
    cout << "Memory Contents:" << endl;
    for (int i = 0; i < MEM_SIZE; ++i)
    {
        cout << "Block " << i << ": " << memory[i] << endl;
    }
    cout << endl;
}

void initializeMemory()
{
    for (int i = 0; i < MEM_SIZE; ++i)
    {
        memory[i] = i + 5;
    }
}

void displayCache()
{
    cout << "Cache Contents:" << endl;
    for (int i = 0; i < CACHE_SIZE / CACHE_ASSOC; ++i)
    {
        for (int j = 0; j < CACHE_ASSOC; ++j)
        {
            cout << "Set " << i << ", Block " << j << ": ";
            if (cache[i][j].valid)
            {
                cout << " H:" << cache[i][j].history;
                cout << " V:1 Tag:" << cache[i][j].tag << " Data:" << cache[i][j].data;
            }
            else
            {
                cout << " H:" << cache[i][j].history;
                cout << " V:0";
            }
            cout << endl;
        }
    }
    cout << endl;
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
    for (int i = 0; i < 8; ++i)
    {
        cout << "$s" << i << ": " << registers[i] << endl;
    }
    cout << endl;
}

// find least recently used block
int findLRUBlock(int index)
{
    int block = -1;
    if (cache[index][0].history == 1 && cache[index][1].history == 1)
    {
        cout << "error in findLRUBlock()" << endl;
        exit(1);
    }

    // cold start, choose block 0
    if (cache[index][0].history == 0 && cache[index][1].history == 0)
    {
        block = 0;
        cache[index][1].history = 1;
    }
    // choose block 0
    else if (cache[index][0].history == 1)
    {
        block = 0;
    }
    // choose block 1
    else if (cache[index][1].history == 1)
    {
        block = 1;
    }
    return block;
}

void updateHistoryBits(int index, int block)
{
    if (block == 0)
    {
        cache[index][0].history = 0;
        cache[index][1].history = 1;
    }
    else if (block == 1)
    {
        cache[index][0].history = 1;
        cache[index][1].history = 0;
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