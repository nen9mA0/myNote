#include <iostream>
#include <vector>

using namespace std;

int main(void)
{
	int cache;

	vector<int> input;
	while(cin>>cache)
		input.push_back(cache);

	for(vector<int>::iterator iter=input.begin();iter!=input.end();iter++)
		cout<<*iter<<endl;

	return 0;
}
