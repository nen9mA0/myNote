#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>

using namespace std;

int main()
{
    vector< pair<int, int> > vec;

    for(int i=0; i<10; i++)
    {
        vec.push_back(make_pair(i, i));
    }

    multimap<int, int> mm(vec.begin(), vec.end());

    cout << "mm first init:" << endl;
    for(multimap<int, int>::iterator i=mm.begin(); i!=mm.end(); i++)
        cout << i->first << " : " << i->second << endl;

    vec.clear();
    for(int i=0; i<10; i++)
    {
        vec.push_back(make_pair(i, i+10));
    }

    mm.insert(vec.begin(), vec.end());

    cout << "mm insert:" << endl;
    for(multimap<int, int>::iterator i=mm.begin(); i!=mm.end(); i++)
        cout << i->first << " : " << i->second << endl;

    int key = 1;
    cout << "mm find key " << key << endl;
    cout << "use find and count" << endl;
    multimap<int, int>::size_type size = mm.count(key);
    if( size )
    {
        multimap<int, int>::iterator iter = mm.find(key);
        for(multimap<int, int>::size_type i=0; i<size; i++, iter++)
            cout << iter->first << " : " << iter->second << endl;
    }

    key = 2;
    cout << "mm find key " << key << endl;
    cout << "use lower_bound and upper_bound" << endl;

    for(multimap<int, int>::iterator i=mm.lower_bound(key); i!=mm.upper_bound(key); i++)
        cout << i->first << " : " << i->second << endl;

    key = 100;
    cout << "mm find key " << key << endl;

    pair< multimap<int, int>::iterator, multimap<int, int>::iterator > range = mm.equal_range(key);
    while(range.first != range.second)
    {
        cout << range.first->first << " : " << range.first->second << endl;
        range.first++;
    }
 }