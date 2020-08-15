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
    vector<int> vec;
    istream_iterator<int> input(cin);
    istream_iterator<int> end;

    while(input != end)
    {
        vec.push_back(*input++);
    }

    for(vector<int>::iterator i=vec.begin(); i!=vec.end(); i++)
        cout << *i << endl;

}