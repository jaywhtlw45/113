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
void initializeCache();

void displayCache();
void displayMemory();
void accessMemory(int address, bool write);

void processInstruction(bitset<BITS> instruction);
bitset<BITS> stringToBitset(string line);
void readFile();

// Register file
int registers[8] = {0};

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
    bitset<6> opcode;   // bits 26-31
    for(int i = 0; i < 6; i++) {
        opcode[i] = instruction[i+26];
    }
    bitset<5> rs;       // bits 21-25
    for(int i= 0; i < 5; i++) {
        rs[i] = instruction[i+21];
    }
    bitset<5> rt;       // bits 16-20
    for(int i = 0; i < 5; i++) {
        rt[i] = instruction[i+16];
    }
    bitset<16> offset;  // bits 0-15
    for(int i = 0; i < 16; i++) {
        offset[i] = instruction[i];
    }

    if (opcode == 35) {
        // load
        cout << "load" << endl;
        cout << "rs " << rs << endl;
        cout << "rt " << rt << endl;
        cout << "offset " << offset << endl;
    } else if (opcode == 43) {
        // store
        cout << "store" << endl;
        cout << "rs " << rs << endl;
        cout << "rt " << rt << endl;
        cout << "offset " << offset << endl;
    } else {
        cout << "opcode " << opcode << endl;
    }
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

void initializeCache()
{

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
                cout << "V:1 Tag:" << cache[i][j].tag << " Data:" << cache[i][j].data;
            }
            else
            {
                cout << "V:0";
            }
            cout << endl;
        }
    }
    cout << endl;
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

// Function to simulate memory access
void accessMemory(int address, bool write)
{
    // Extract index and tag from the address
    int index = (address >> 2) & 0x07; // 3 bits for index
    bitset<4> tag(address >> 5);       // 4 bits for tag

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
    cout << (cacheHit ? "Hit" : "Miss") << " at address " << address << endl;

    // If it's a cache miss, update the cache
    if (!cacheHit)
    {
        // Find the LRU way
        way = (cache[index][0].valid && cache[index][1].valid) ? 0 : 1;

        // Update the cache block
        cache[index][way].valid = true;
        cache[index][way].tag = tag;
        cache[index][way].data = memory[address >> 2];
    }

    // If it's a write operation, update the data in the cache and memory
    if (write)
    {
        int regIndex = (address >> 2) & 0x07; // 3 bits for register index
        cache[index][way].data = registers[regIndex];
        memory[address >> 2] = registers[regIndex];
    }
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