#include <iostream>

using namespace std;

int main()
{
    for(int i=0; i<30; i++)
    {
        string tmp;
        for(int j=0; j<i; j++)
            tmp += 'A';

        unsigned long long str_begin = reinterpret_cast<unsigned long long>(&tmp);
        unsigned long long str_end = str_begin + sizeof(string);

        const char* cstr = tmp.c_str();
        unsigned long long cstr_tmp = reinterpret_cast<unsigned long long>(cstr);
        if( !(cstr_tmp > str_begin && cstr_tmp < str_end) )
        {
            cout << "i: " << i << endl;
            cout << "cstr_tmp: " << (void*)cstr_tmp << endl;
            cout << "str_begin: " << (void*)str_begin << endl;
            cout << "str_end: " << (void*)str_end << endl;
        }
    }
}