#include <iostream>
#include <vector>
#include <bitset>

using namespace std;

int main(void)
{
	string str("11010010");
	bitset<32> b(str);

	cout<<".any"<<endl<<b.any()<<endl;
	cout<<".none"<<endl<<b.none()<<endl;
	cout<<".count"<<endl<<b.count()<<endl;
	cout<<".size"<<endl<<b.size()<<endl;
	cout<<"b[2]"<<endl<<b[2]<<endl;
	cout<<".test(2)"<<endl<<b.test(2)<<endl;
	cout<<".set"<<endl<<b.set()<<endl;
	b(str);
	cout<<b<<endl;

	return 0;
}
