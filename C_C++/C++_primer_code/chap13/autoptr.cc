#include <iostream>
#include <string>

#define _DEBUG

using namespace std;

class Autoptr
{
private:
    friend class Ptr;
    Autoptr(int *ptr): p(ptr), use(1) {  }
    ~Autoptr();

    int *p;
    int use;
};

class Ptr
{
public:
    Ptr(int *ptr): use_reference(new Autoptr(ptr)) {  }
    Ptr(const Ptr&);
    Ptr& operator=(const Ptr&);
    ~Ptr();

    void PntRef();
private:
    Autoptr* use_reference;
};

Autoptr::~Autoptr()
{
    delete p;
#ifdef _DEBUG
    cout << "!Autoptr delete p: " << p << endl;
#endif
}

Ptr::Ptr(const Ptr& rhs): use_reference(rhs.use_reference)
{
    use_reference->use++;
#ifdef _DEBUG
    cout << use_reference << " reference add to " << use_reference->use << endl;
#endif
}

Ptr& Ptr::operator=(const Ptr& rhs)
{
    if( !(--use_reference->use) )
    {
        delete use_reference;
#ifdef _DEBUG
        cout << "operator= delete use_reference: " << use_reference << endl;
#endif
    }
    use_reference = rhs.use_reference;
    use_reference->use++;

#ifdef _DEBUG
    cout << use_reference << " reference add to " << use_reference->use << endl;
#endif

    return *this;
}

Ptr::~Ptr()
{
    if( !(--use_reference->use) )
    {
        delete use_reference;
#ifdef _DEBUG
        cout << "~Ptr delete: " << use_reference << endl;
#endif
    }
    else
    {
#ifdef _DEBUG
        cout << use_reference << " reference sub to " << use_reference->use << endl;
#endif
    }
}

void Ptr::PntRef()
{
    cout << "p: " << use_reference->p << " \t use: " << use_reference->use << endl;
}

void test(int *p)
{
    Ptr ptr1(p);
    Ptr ptr2(ptr1);
    Ptr ptr3(ptr1);
    Ptr ptr4(ptr2);

    ptr4.PntRef();

    Ptr ptr5 = ptr4;

    ptr5.PntRef();
}

int main()
{
    int *p = new int(80);

    cout << *p << endl;
    test(p);
    cout << *p << endl;     //deleted

    return 0;
}