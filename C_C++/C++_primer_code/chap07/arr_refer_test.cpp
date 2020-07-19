#include <iostream>

using namespace std;

void print_arr(int (&arr)[10])
{
    for(int i=0; i<10; i++)
        cout << arr[i] << endl;
}

int main()
{
    int *a = new int [10];
    int b[10] = {0};
    print_arr(b);
}
