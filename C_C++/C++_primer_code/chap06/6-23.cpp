#include <iostream>
#include <bitset>
#include <stdexcept>

using namespace std;

int main(void)
{
    bitset<64> b;
    string num;
    unsigned char a;

    cin >> num;
    b = bitset<64>(num,0);
    try
    {
        a = b.to_ulong();
        cout<<b<<endl<<a<<endl;
    }
    catch(runtime_error err)
    {
        cout<<err.what()<<endl;
    }

    return 0;
}