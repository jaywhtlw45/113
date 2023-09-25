#include <iostream>
using namespace std;

// bitset header

// Input: a, b, carry_in, operation
// Output: result, carry_out
void bit_add(int a, int b, int carry_in, int &sum, int &carry_out)
{
    sum = a ^ b ^ carry_in;
    carry_out = (a & b) | (a ^ b) & carry_in;
}

void bit_sub(int a, int b, int carry_in, int &sum, int &carry_out)
{
    // perform 2's complement on a b
    if (b == 0) // flip
        b = 1;
    else if (b == 1)
        b = 0;

    if (b == 0) // add 1
        b = 1; 
    else if (b == 1)
    {
        b == 0;
        carry_in += 1;
    }

    bit_add(a, b, carry_in, sum, carry_out);

    add(a, add(~b, 1));
}
int main()
{

    int a = 0;
    int b = 0;
    int carry_in = 0;
    int sum;
    int carry_out;

    bit_add(a, b, carry_in, sum, carry_out);
    cin.get();
    return 0;
}