// Jason Whitlow
// CSCI 113 Program 4
// Booth's Algorithm

// This program takes two binary numbers and multiplies them together using Booth's Algorithm.

#include <iostream>
#include <string>
#include <bitset>
#include <iomanip>
using namespace std;

const int ALU_SIZE = 16;

// One bit addition alu
int add_one_bit(int a, int b, int carry_in, int &carry_out)
{
    carry_out = (a & b) | (a ^ b) & carry_in;
    return a ^ b ^ carry_in;
}

// Multiple bit addition alu
void add_many_bits(bitset<ALU_SIZE> a, bitset<ALU_SIZE> b, int borrow_in, bitset<ALU_SIZE> &dif, int &borrow)
{
    for (int i = 0; i < ALU_SIZE; i++)
    {
        dif[i] = add_one_bit(a[i], b[i], borrow_in, borrow);
        borrow_in = borrow;
    }
}

// One bit subtraction alu
int sub_one_bit(int a, int b, int carry_in, int &carry_out)
{
    carry_out = ((!a) & b) | (!(a ^ b) & carry_in);
    return (a ^ b) ^ carry_in;

    // add(a, add(~b, 1));
}

// Multiple bit subtraction alu
void sub_many_bits(bitset<ALU_SIZE> a, bitset<ALU_SIZE> b, int carry_in, bitset<ALU_SIZE> &sum, int &carry_out)
{
    for (int i = 0; i < ALU_SIZE; i++)
    {
        sum[i] = sub_one_bit(a[i], b[i], carry_in, carry_out);
        carry_in = carry_out;
    }
}

// Booth's Algorithm takes multiplicand and multiplier as input.
// It then multiplies them together and outputs the result.
// ac = accumulator, md = multiplicand, mq = multiplier
void booths_alg(bitset<ALU_SIZE> md, bitset<ALU_SIZE> mq)
{
    bitset<ALU_SIZE> ac = 0;
    bitset<ALU_SIZE> temp = 0;

    int mq_neg1 = 0;        // mq(-1)
    int store_bit;          // temporary storage for shifting
    int counter = ALU_SIZE; // counter for number of cycles

    int carry_in, carry_out;

    cout
        << "md: " << md << endl
        << "mq: " << mq << endl
        << "----------------" << endl
        << "cycle" << setw(ALU_SIZE + 4) << "md" << setw(ALU_SIZE + 4) << "ac" << setw(ALU_SIZE + 4) << "mq" << setw(ALU_SIZE + 4) << "mq(-1)" << endl;

    while (counter > 0)
    {
        // Print out current cycle
        cout << counter << setw(ALU_SIZE + 10) << md << setw(ALU_SIZE + 4) << ac << setw(ALU_SIZE + 4) << mq << setw(ALU_SIZE) << mq_neg1 << endl;

        // ac = ac + md
        if (mq[0] == 0 && mq_neg1 == 1)
        {
            add_many_bits(ac, md, carry_in, temp, carry_out);
            ac = temp;
        }
        // ac = ac - md
        else if (mq[0] == 1 && mq_neg1 == 0)
        {
            sub_many_bits(ac, md, carry_in, temp, carry_out);
            ac = temp;
        }

        // shift mq right
        mq_neg1 = mq[0];
        mq = mq >> 1;
        mq[ALU_SIZE - 1] = ac[0];

        // shift ac right
        store_bit = ac[ALU_SIZE - 1];
        ac = ac >> 1;
        ac[ALU_SIZE - 1] = store_bit;

        counter--;
    }

    // Print out final cycle
    cout << counter << setw(ALU_SIZE + 10) << md << setw(ALU_SIZE + 4) << ac << setw(ALU_SIZE + 4) << mq << setw(ALU_SIZE + 4) << mq_neg1 << endl;
    cout << "Result: " << ac << " " << mq << endl
         << endl;
}

int main()
{
    // Run Booth's Algorithm
    cout << endl
         << "Booth's Algorithm" << endl
         << endl;

    bitset<ALU_SIZE> md1("0000001000011011"); // 539
    bitset<ALU_SIZE> mq1("1111111111100110"); // -26
    booths_alg(md1, mq1);                     // Result: -14014

    bitset<ALU_SIZE> md2("1111111111100110"); // -26
    bitset<ALU_SIZE> mq2("0000001000011011"); // 539
    booths_alg(md2, mq2);                     // Result: -14014

    bitset<ALU_SIZE> md3("1111111110101010"); // -86
    bitset<ALU_SIZE> mq3("1111111110110100"); // -76
    booths_alg(md3, mq3);                     // Result: 6536
    return 0;
}
