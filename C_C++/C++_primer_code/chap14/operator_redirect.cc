#include <iostream>

class TestBase
{
public:
    TestBase() {  }
    virtual std::ostream& display(std::ostream&) const = 0;
};

class TestA: public TestBase
{
    friend std::ostream& operator<<(std::ostream& os, const TestA& t);
public:
    TestA(int n): num(n) {  }
    std::ostream& display(std::ostream& os) const { return os << "TestA num: " << num; }
private:
    int num;
};

std::ostream& operator<<(std::ostream& os, const TestA& t)
{
    return t.display(os);
}

int main()
{
    TestA a(10);
    std::cout << a << std::endl;
}