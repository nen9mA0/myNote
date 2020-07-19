#include <iostream>
#include <vector>
#include <stack>

using namespace std;

int main()
{
    char *word[] = {"abc","abcd","abcde"};
    size_t p = sizeof(word)/sizeof(char*);
    vector<string> mystr(word,word+p);

    for(vector<string>::iterator i= mystr.begin();i!=mystr.end();i++)
        cout<<*i<<endl;


    vector<int> v(5, 1);
    vector<int>::iterator first = v.begin();

    while(first != v.end())
    {
        first = v.insert(first, 42);
        ++first;
        ++first;
    }

    for(vector<int>::iterator i = v.begin(); i < v.end(); i++)
        cout << *i << endl;

    vector<int> capacity_test(1, 1);
    capacity_test.reserve(20);
    int tmp = capacity_test.capacity();
    cout << "capacity_test capacity: " << capacity_test.capacity() << "\tcapacity_test size: " << capacity_test.size() << endl;
    for(int i=0; i<tmp; i++)
        capacity_test.push_back(1);

    cout << "capacity_test capacity: " << capacity_test.capacity()  << "\tcapacity_test size: " << capacity_test.size() << endl;

    capacity_test.push_back(1);
    cout << "capacity_test capacity: " << capacity_test.capacity()  << "\tcapacity_test size: " << capacity_test.size() << endl;

    deque<int> d1;

    for(int i=0; i<10; i++)
        d1.push_back(i);

    stack<int> stk1(d1);

    stk1.pop();
    stk1.push(10);
    while(!stk1.empty())
    {
        cout << stk1.top() << endl;
        stk1.pop();
    }

    cout << endl;
    for(deque<int>::iterator i=d1.begin(); i!=d1.end(); i++)
        cout << *i << endl;
}