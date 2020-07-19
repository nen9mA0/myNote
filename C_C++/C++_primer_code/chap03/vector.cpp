#include <iostream>
#include <vector>
using namespace std;

int main(void)
{
	vector<int> a(10,1);
	vector<string> str(2,"hello");

	for(int i=0;i<10;i++)
	{
		cout<<a[i];
	}
	str[1]="World";
	cout<<str[0]<<endl;
	cout<<str[1]<<endl;	
	return 0;
}
