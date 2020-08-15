#include <iostream>
#include <string>
#include <cstring>

//#define _MYSTR_DEBUG
//#define _TEST_DEBUG

using namespace std;

const char longstr[] = "0123456789abcdef01";

class Mystr
{
public:
    Mystr();
    Mystr(const char *str);
    Mystr(const Mystr &);
    ~Mystr();

    void print();
private:
    union data
    {
        char str[16];
        char* p;
    };
    data mydata;
    size_t length;
};

Mystr::Mystr(): mydata{0}, length(0)
{
#ifdef _MYSTR_DEBUG
    cout << "====Mystr default constructor" << endl;
    cout << "==== data: " << mydata.str << endl;
    cout << "==== length: " << length << endl;
    cout << "==== pointer: " << this << endl;
#endif
}

Mystr::Mystr(const char *str)
{
#ifdef _MYSTR_DEBUG
    cout << "====Mystr char* constructor" << endl;
#endif

    size_t len = strlen(str);
    if(len >= 16)
    {
        char* tmp_p = new char[len+1];
        mydata.p = tmp_p;
        memcpy(tmp_p, str, len+1);
#ifdef _MYSTR_DEBUG
        cout << "==== data: " << mydata.p << endl;
#endif
    }
    else
    {
        memcpy(mydata.str, str, len+1);
#ifdef _MYSTR_DEBUG
        cout << "==== data: " << mydata.str << endl;
#endif
    }
    length = len;

#ifdef _MYSTR_DEBUG
    cout << "==== length: " << length << endl;
    cout << "==== pointer: " << this << endl;
#endif
}

Mystr::Mystr(const Mystr& src)
{
#ifdef _MYSTR_DEBUG
    cout << "====Mystr copy constructor" << endl;
#endif

    if(src.length >= 16)
    {
        char* tmp_p = new char[src.length+1];
        memcpy(tmp_p, src.mydata.p, src.length+1);
        mydata.p = tmp_p;
#ifdef _MYSTR_DEBUG
        cout << "==== data: " << mydata.p << endl;
#endif
    }
    else
    {
        memcpy(mydata.str, src.mydata.str, src.length+1);
#ifdef _MYSTR_DEBUG
        cout << "==== data: " << mydata.str << endl;
#endif
    }
    length = src.length;

#ifdef _MYSTR_DEBUG
    cout << "==== length: " << length << endl;
    cout << "==== source pointer: " << &src << endl;
    cout << "==== destin pointer: " << this << endl;
#endif
}

Mystr::~Mystr()
{
#ifdef _MYSTR_DEBUG
    cout << "==== Mystr destructor" << endl;
    cout << "pointer: " << this << endl;
#endif
    if(length >= 16)
    {
        delete [] mydata.p;
    }
}

void Mystr::print()
{
    cout << "====== Mystr print ======" << endl;
    if(length >= 16)
    {
        cout << "data: " << mydata.p << endl;
    }
    else
    {
        cout << "data: " << mydata.str << endl;
    }
    cout << "length: " << length << endl;
}

class Test
{
public:
    Test(string str);
    Test(const char* str);
    Test(const Test &);
    Test();
    ~Test();

    void print();
    void setdata(string str);
    void setmystr(const char* str);

private:
    string data;
    int length;
    Mystr mystr;
};

Test::Test() : data(""), length(0), mystr()
{
#ifdef _TEST_DEBUG
    cout << "==Test default constructor" << endl;
    cout << "== data: " << data << endl;
    cout << "== length: " << length << endl;
    cout << "== pointer: " << this << endl;
#endif
}

Test::Test(string str): data(str), length(str.size()), mystr()
{
#ifdef _TEST_DEBUG
    cout << "==Test string constructor" << endl;
    cout << "== data: " << data << endl;
    cout << "== length: " << length << endl;
    cout << "== pointer: " << this << endl;
#endif
}

Test::Test(const char* str): mystr(str), data(""), length(0)
{
#ifdef _TEST_DEBUG
    cout << "==Test char* constructor" << endl;
    cout << "== pointer: " << this << endl;
#endif
}

Test::Test(const Test &tst)
{
    data = tst.data;
    length = tst.length;
#ifdef _TEST_DEBUG
    cout << "==Test copy constructor" << endl;
    cout << "== data: " << data << endl;
    cout << "== length: " << length << endl;
    cout << "== source pointer: " << &tst << endl;
    cout << "== destin pointer: " << this << endl;
#endif
}

Test::~Test()
{
#ifdef _TEST_DEBUG
    cout << "== Test destructor ==" << endl;
    cout << "pointer: " << this << endl;
#endif
}

void Test::setdata(string str)
{
#ifdef _TEST_DEBUG
    cout << "== Test setdata ==" << endl;
    cout << "pointer: " << this << endl;
#endif
    data = str;
}

void Test::setmystr(const char* str)
{
#ifdef _TEST_DEBUG
    cout << "== Test setmystr ==" << endl;
    cout << "pointer: " << this << endl;
#endif
    mystr = Mystr(str);
}

void Test::print()
{
    cout << "====== Test print ======" << endl;
    cout << "pointer: " << this << endl;
    cout << "data: " << data << endl;
    cout << "length: " << length << endl;
    mystr.print();
}

Test changedata(Test tst)
{
    cout << "changedata call" << endl;
    tst.setdata(string(longstr));
    cout << "setdata call end" << endl;
    return tst;
}

Test changemystr(Test tst)
{
    cout << "changemystr call" << endl;
    tst.setmystr(longstr);
    return tst;
}

Test tst_arr[5] = {Test("1"), Test("2"), string("3"), "4"};

int main()
{
    cout << "================ string test ================" << endl;
    Test tst = string("1234"); // implicit type conversion
    tst.print();
    //Test tst2 = change(tst);
    //tst2.print();
    tst = changedata(tst);
    tst.print();

    cout << "tst pointer: " << &tst << endl;

    cout << "================ Mystr test ================" << endl;
    /*
    tst = Test("1234");
    tst.print();
    tst = changemystr(tst);
    tst.print();
    */
    Test tst1("1234");
    Test tst2;
    
    tst1.print();
    tst2 = changemystr(tst1);
    tst1.print();
    tst2.print();

    cout << "================ End Test ================" << endl;
}
