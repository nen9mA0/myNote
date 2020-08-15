#include <iostream>

using namespace std;

class Test
{
public:
    Test(): a(), b() {  }
    Test(int ina, int inb): a(ina), b(inb) {  }

    //Test& operator+(const Test&);
    Test operator+(const Test&);

    int a;
    int b;
};

/*
Test& Test::operator+(const Test& rhs)
{
    Test* p = new Test(a, b);
    p->a += rhs.a;
    p->b += rhs.b;
    return *p;
}
*/

Test Test::operator+(const Test& rhs)
{
    Test tst(a+rhs.a, b+rhs.b);
    return tst;
}


int main()
{
    Test tst1(10, 20);
    Test tst2(12, 22);

    Test tst3 = tst1 + tst2;
    cout << tst3.a << "  " << tst3.b << endl;

    Test tst4 = tst1.operator+(tst2);
    cout << tst4.a << "  " << tst4.b << endl;
    return 0;
}