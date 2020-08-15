#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>

using namespace std;

int main()
{
    vector< pair<string, int> > pair_vec;
    map<string, int> m;
    stringstream ss;

    for(int i=0; i<10; i++)
    {
        string key;
        ss.clear();
        ss << i;
        ss >> key;
        pair_vec.push_back(make_pair(key, i));
    }

    vector< pair<string, int> >::iterator beg, end;

    beg = pair_vec.begin();
    end = pair_vec.end();

    m.insert(beg, end);

    for(map<string, int>::iterator i=m.begin(); i!=m.end(); i++)
        cout << (*i).first << " : " << (*i).second << endl;

    map<string, int> m1;
    map<string, int>::iterator iter = m.begin();
    for(int i=0; i<5; i++)
        iter++;

    m1.insert(m.begin(), iter);

    cout << "m1:" << endl;
    for(map<string, int>::iterator i=m1.begin(); i!=m1.end(); i++)
        cout << (*i).first << " : " << (*i).second << endl;

    string new_key("10");
    pair< map<string, int>::iterator, bool > ret = m1.insert(make_pair(new_key, 10));
    cout << "insert  " << new_key << " : " << 10 << endl;
    cout << (ret.first)->first << " : " << (ret.first)->second << "  bool:  " << ret.second << endl;

    for(map<string, int>::iterator i=m1.begin(); i!=m1.end(); i++)
        cout << (*i).first << " : " << (*i).second << endl;
}