#include <iostream>

class Father
{
public:
    Father(): num(0), ch('0') { std::cout << "construct Father" << std::endl; }
    Father(int n, char c): num(n), ch(c) { std::cout << "construct Father with param" << std::endl; }
    int GetNum() const { return num; }
    virtual char GetChar() const { std::cout << "call from father" << std::endl; return ch; }
    virtual ~Father() { std::cout << "deconstruct Father" << std::endl; }
    
    std::string name;
private:
    int num;
protected:
    char ch;
};

class ChildA: public Father
{
public:
    ChildA() {  }
    ChildA(int n, char c): Father(n, c) {  }
    char GetChar() const { std::cout << "call from ChildA" << std::endl; return ch; }
    ~ChildA() { std::cout << "deconstruct ChildA" << std::endl; }
};

class ChildB: public Father
{
public:
    ChildB() { std::cout << "construct ChildB" << std::endl; }
};

int main()
{
    ChildA a(1, 'a');
    ChildB b;

    std::cout << "ChildA" << std::endl;
    std::cout << a.GetChar() << std::endl;
    std::cout << a.GetNum() << std::endl;

    std::cout << "ChildB" << std::endl;
    std::cout << b.GetChar() << std::endl;
    std::cout << b.GetNum() << std::endl;
}