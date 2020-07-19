#include <iostream>

using namespace std;

int print(string);
int print(int);
int print(char);
int test_fun(void);

int test(int a, int b);
int test(int a, int b, int c=0);

int main(void)
{
    test_fun();
    return 0;
}

int print(string str)
{
    cout<<str<<endl;
}

int print(int num)
{
    cout<<"num is "<<num<<endl;
}

int print(char ch)
{
    cout<<ch;
}

int test_fun()
{
    int a = 1;
    string str("string test");
    int print(int);
    print(a);
    test(1,2);
}

int test(int a, int b)
{
    cout << "2 param" << endl;
}

int test(int a, int b, int c)
{
    cout << "3 param" << endl;
}