#include <iostream>
#include <string>
#include <bitset>
#include <iomanip>
using namespace std;

const int ALU_SIZE = 4;

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

void booths_alg(bitset<ALU_SIZE> md, bitset<ALU_SIZE> mq)
{
    bitset<ALU_SIZE> ac = 0;
    int mq_neg1 = 0;
    int counter = ALU_SIZE;

    int carry_in, carry_out;

    cout << "Booth's Algorithm" << endl
         << "----------------" << endl;

    while (counter < 0)
    {
        if (mq[0] == 0 && mq_neg1 == 1)
        {
            add_many_bits(ac, md, carry_in, ac, carry_out);
        }
        else if (mq[0] == 1 && mq_neg1 == 0)
        {
            sub_many_bits(ac, md, carry_in, ac, carry_out);
        }

        mq_neg1 = mq[0];
        mq = mq >> 1;
        mq[ALU_SIZE - 1] = ac[0];
        ac = ac >> 1;
        counter--;

        cout << "cycle" << setw(ALU_SIZE + 4) << "md" << setw(ALU_SIZE + 4) << "ac" << setw(ALU_SIZE + 4) << "mq" << setw(ALU_SIZE + 4) << "mq(-1)";
    }
}

int main()
{
    // Addition ALU
    bitset<ALU_SIZE> a("1010");
    bitset<ALU_SIZE> b("0101");
    int carry_in = 0;
    bitset<ALU_SIZE> sum;
    int carry_out;

    add_many_bits(a, b, carry_in, sum, carry_out);

    cout << "Addition ALU" << endl
         << "----------------" << endl;
    cout << "a:          " << a << endl;
    cout << "b:          " << b << endl;
    cout << "carry_in:   " << carry_in << endl;
    cout << "a + b:      " << sum << endl;
    cout << "carry_out:  " << carry_out << endl
         << endl
         << endl;

    // Subtraction ALU
    bitset<ALU_SIZE> c("0010");
    bitset<ALU_SIZE> d("0001");
    int borrow_in = 0;
    bitset<ALU_SIZE> diff;
    int borrow_out;

    sub_many_bits(c, d, borrow_in, diff, borrow_out);

    cout << "Subtraction ALU" << endl
         << "----------------" << endl;
    cout << "c:          " << c << endl;
    cout << "d:          " << d << endl;
    cout << "borrow_in:  " << borrow_in << endl;
    cout << "a - b:      " << diff << endl;
    cout << "borrow_out: " << borrow_out << endl
         << endl;

    // Booth's Algorithm
    bitset<ALU_SIZE> md("0010");
    bitset<ALU_SIZE> mq("0001");

    booths_alg(md, mq);

    return 0;
}
