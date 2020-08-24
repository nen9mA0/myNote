#include <iostream>

class ClassBase
{
public:
    virtual int calc() = 0;
    virtual ~ClassBase() {  }
};

class ItemA : public ClassBase
{
public:
    int calc() { return 1; }
    ~ItemA() {  }
};

class ItemB : public ClassBase
{
public:
    int calc() { return 2; }
    ~ItemB() {  }
};

class Handler
{
public:
    Handler(): use(new int(1)), class_ptr(nullptr) {  }
    Handler(ClassBase& p): use(new int(1)), class_ptr(&p) {  }
    Handler(const Handler& h): use(h.use), class_ptr(h.class_ptr) { *use++; }
    Handler& operator=(Handler& rhs) { *(rhs.use)++; DecrUse(); use = rhs.use; class_ptr = rhs.class_ptr; return *this; }
    ClassBase& operator*() const { if(class_ptr) { return *class_ptr; } else { throw std::logic_error("NULL REF"); } }
    ClassBase* operator->() const { if(class_ptr) { return class_ptr; } else { throw std::logic_error("NULL Pointer"); } }
    ~Handler() { DecrUse(); }
private:
    void DecrUse() { if(--*use == 0) { delete class_ptr; delete use; } }

    int *use;
    ClassBase* class_ptr;
};

int AddItem(ClassBase& a, ClassBase& b)
{
    return a.calc() + b.calc();
}

int AddItemHandler(Handler a, Handler b)
{
    return a->calc() + b->calc();
}

int main()
{
    ItemA a;
    ItemB b;

    std::cout << AddItem(a, b) << std::endl;

    try
    {
        Handler ha(a);
        Handler hb(b);

        std::cout << AddItemHandler(ha, hb) << std::endl;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

}