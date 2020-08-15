#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <iterator>

using namespace std;

int main()
{
    vector<int> vec;
    for(int i=0; i<5; i++)
        vec.push_back(i);
    fill_n(back_inserter(vec), 10, 100);
    for(vector<int>::iterator i=vec.begin(); i!=vec.end(); i++)
        cout << *i << endl;
}