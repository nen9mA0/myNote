#include <iostream>
#include <vector>
#include <bitset>
#include <string>

int main(void)
{
	std::string str = "11010010";
	std::bitset<8> b(str);

	std::cout<<".any"<<std::endl<<b.any()<<std::endl;
	std::cout<<".none"<<std::endl<<b.none()<<std::endl;
	std::cout<<".count"<<std::endl<<b.count()<<std::endl;
	std::cout<<".size"<<std::endl<<b.size()<<std::endl;
	std::cout<<"b[2]"<<std::endl<<b[2]<<std::endl;
	std::cout<<".test(2)"<<std::endl<<b.test(2)<<std::endl;
	std::cout<<".set"<<std::endl<<b.set()<<std::endl;
	b = 0x1;
	std::cout<<b<<std::endl;

	return 0;
}
