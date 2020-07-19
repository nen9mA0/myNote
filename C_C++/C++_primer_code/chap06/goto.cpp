#include <iostream>
#include <malloc.h>

using namespace std;

int main()
{
    int flag=0;
begin:
    int b = 2;
   // int *p = (int*)alloca(8);
    static int a=1;
    a++;
    cout<<"pointer to b:"<<&b<<endl;
    int *p = (int*)alloca(8);
    cout<<"pointer to alloca:"<<p<<endl;
    if(a!=10)
        goto begin;
    cout<<"Free with:"<<a<<endl;

}