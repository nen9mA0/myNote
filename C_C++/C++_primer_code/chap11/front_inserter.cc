#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <iterator>

using namespace std;

int main()
{
    list<int> vec1;
    for(int i=0; i<10; i++)
        vec1.push_back(i);
    
    list<int> vec2;
    copy(vec1.begin(), vec1.end(), front_inserter(vec2));

    list<int> vec3;
    copy(vec1.begin(), vec1.end(), inserter(vec3, vec3.begin()));

    cout << "vec2:" << endl;
    for(list<int>::iterator i=vec2.begin(); i!=vec2.end(); i++)
        cout << *i << endl;

    cout << "vec3:" << endl;
    for(list<int>::iterator i=vec3.begin(); i!=vec3.end(); i++)
        cout << *i << endl;
}