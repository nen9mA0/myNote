#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main(void)
{
	string str1="abc";
	string str2(str1);
	string str3(3,'2');
	string input,uiput,before;

	cout<<str1<<endl<<str2<<endl<<str3<<endl;

	cin>>input;
	cout<<input;

	while(cin>>uiput)
	{
		if(uiput==before)
			cout<<"Same"<<endl<<uiput.size()<<endl;
		else
			cout<<uiput<<endl<<before+uiput<<endl;
		before=uiput;
	}
	return 0;
}
