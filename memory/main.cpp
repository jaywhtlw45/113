#include <iostream>
#include <bitset>
#include <vector>
#include <fstream>
#include <string>

using namespace std;

// Cache block structure
struct CacheBlock
{
    bool valid;
    bool coldStart;
    bool history;
    bitset<4> tag;
    int data;
};

// Main memory size
const int MEM_SIZE = 128;

// Cache size and associativity
const int CACHE_SIZE = 16;
const int CACHE_ASSOC = 2;
const int BITS = 32;

// Cache structure
vector<vector<CacheBlock>> cache(CACHE_SIZE / CACHE_ASSOC, vector<CacheBlock>(CACHE_ASSOC));

// Function prototypes
void initializeMemory();
void displayCache();
void displayMemory();
void accessMemory(bitset<16> address, bool write);

void processInstruction(bitset<BITS> instruction);

bitset<BITS> stringToBitset(string line);
void readFile();

// Register file
int registers[8] = {0};

// Cache bits
int coldStartBit[8] = {0};
int historyBit[8] = {0};

// Main memory
int memory[MEM_SIZE];

int main()
{
    // Initialize memory, cache, and registers
    initializeMemory();
    readFile();
    displayCache();

    // Display final contents
    // displayCache();
    // displayMemory();

    return 0;
}

// Function to read the instruction
// '100011' for load and '101011' for store
void processInstruction(bitset<BITS> instruction)
{
    bitset<6> opcode; // bits 26-31
    for (int i = 0; i < 6; i++)
    {
        opcode[i] = instruction[i + 26];
    }
    bitset<16> address;
    for (int i = 0; i < 16; i++)
    {
        address[i] = instruction[i];
    }

    int int_address = address.to_ulong();
    cout << "int_address " << int_address << endl;
    // load
    if (opcode == 35)
    {
        accessMemory(address, 0);
    }
    // store
    else if (opcode == 43)
    {
        accessMemory(address, 1);
    }
    else
    {
        cout << "error" << endl;
        exit(1);
    }
}

int getIndex(bitset<16> address)
{
    bitset<3> index;
    for (int i = 0; i < 4; i++)
        index[i] = address[i];

    return index.to_ulong();
}

bitset<4> getTag(bitset<16> address)
{
    bitset<4> tag;
    for (int i = 3; i < 7; i++)
        tag[i-3] = address[i];

    return tag;
}
// Function to simulate memory access
void accessMemory(bitset<16> address, bool write)
{
    cout << "address " << address << endl;
    // Extract index and tag from the address
    int index = getIndex(address);
    cout << "index " << index << endl;

    bitset<4> tag = getTag(address);
    cout << "tag " << tag << endl;

    int int_address = address.to_ulong();

    // Check if the data is in the cache
    bool cacheHit = false;
    int way = -1;
    for (int i = 0; i < CACHE_ASSOC; ++i)
    {
        if (cache[index][i].valid && cache[index][i].tag == tag)
        {
            cacheHit = true;
            way = i;
            break;
        }
    }

    // Display hit or miss message
    cout << (cacheHit ? "Hit" : "Miss") << " at address " << int_address << endl;

    // If it's a cache miss, update the cache
    if (!cacheHit)
    {
        // Find the LRU way
        way = (cache[index][0].valid && cache[index][1].valid) ? 0 : 1;

        // Update the cache block
        cache[index][way].valid = true;
        cache[index][way].tag = tag;
        cache[index][way].data = memory[int_address >> 2];
    }

    // If it's a write operation, update the data in the cache and memory
    if (write)
    {
        int regIndex = (int_address >> 2) & 0x07; // 3 bits for register index
        cache[index][way].data = registers[regIndex];
        memory[int_address >> 2] = registers[regIndex];
    }
}

void readFile()
{
    ifstream inputFile("input_file.txt");

    if (!inputFile.is_open())
    {
        cerr << "Unable to open file" << endl;
    }

    bitset<BITS> instruction;
    string line;

    while (getline(inputFile, line))
    {
        // instruction = stringToBitset(line);
        instruction = stringToBitset(line);
        processInstruction(instruction);
        break;
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

bitset<BITS> stringToBitset(string line)
{
    bitset<BITS> instruction;
    for (int i = 0; i < BITS; i++)
    {
        if (line[i] == '1')
        {
            instruction.set(BITS - i - 1);
        }
    }
    return instruction;
}

// Input instruction sequence
// int instructions[] = {0x8D010004, 0x8D020010, 0x8D030020, 0x8D040014,
//                       0xAD010050, 0xAD020044, 0xAD03004C, 0xAD0400E0,
//                       0x8D010024, 0x8D02002C, 0x8D030010, 0x8D0400AC,
//                       0xAD010014, 0xAD020018, 0xAD030024, 0xAD040044,
//                       0x8D010024, 0x8D02002C, 0x8D030010, 0x8D0400AC,
//                       0xAD010060, 0xAD020054, 0xAD03005C, 0xAD0400F0};

// Simulate memory operations
// for (int i = 0; i < sizeof(instructions) / sizeof(instructions[0]); ++i) {
//     cout << "instructions " << (instructions[i] & 0x0000FFFF) << endl;
//     accessMemory(instructions[i] & 0x0000FFFF, (instructions[i] >> 29) & 0x1);
// }
