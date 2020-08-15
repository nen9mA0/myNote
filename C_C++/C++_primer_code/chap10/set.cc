#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <set>

using namespace std;

int main()
{
    vector<int> vec;

    for(int i=0; i<10; i++)
    {
        vec.push_back(i);
        vec.push_back(i);
    }

    cout << "vector" << endl;
    for(vector<int>::iterator i=vec.begin(); i!=vec.end(); i++)
        cout << *i << endl;

    cout << "set" << endl;
    set<int> s1(vec.begin(), vec.end());
    for(set<int>::iterator i=s1.begin(); i!=s1.end(); i++)
        cout << *i << endl;             //only if the element is not the same as every elements in the set, it will be added to the set

    return 0;
}