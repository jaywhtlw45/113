#include <iostream>
#include <string>
#include <bitset>
using namespace std;

const int ALU_SIZE = 4;

int single_alu(int a, int b, int carry_in, int &carry_out)
{
    carry_out = (a & b) | (a ^ b) & carry_in;
    cout << "carry_outsa: " << carry_out << endl;
    return a ^ b ^ carry_in;
}

void sub_bits(int a, int b, int carry_in, int &sum, int &carry_out)
{
    sum = (a ^ b) ^ carry_in;
    carry_out = ((!a) & b) | (!(a ^ b) & carry_in);
    // add(a, add(~b, 1));
}

void multiple_alu(bitset<ALU_SIZE> a, bitset<ALU_SIZE> b, int carry_in, bitset<ALU_SIZE> &sum, int &carry_out)
{
    for (int i = 0; i < ALU_SIZE; i++)
    {
        sum[i] = single_alu(a[i], b[i], carry_in, carry_out);
        carry_in = carry_out;
    }
}

int main()
{
    // 16 bit ALU
    bitset<ALU_SIZE> a("1001");
    bitset<ALU_SIZE> b("0001");
    int carry_in = 0;
    bitset<ALU_SIZE> sum;
    int carry_out;

    cout << "a:    " << a << endl;
    cout << "b:    " << b << endl;

    multiple_alu(a, b, carry_in, sum, carry_out);

    cout << "a:     " << a << endl;
    cout << "b:     " << b << endl;
    cout << "c_in:  " << carry_in << endl
         << endl;
    cout << "sum:   " << sum << endl;
    cout << "c_out: " << carry_out << endl
         << endl;

    return 0;
}