### string_storage.cc

测试string的字符串是存放在内存的什么位置的

##### 结果

小于16字节放string中，大于等于则new

### copy_constuct.cc

测试类在各种情况下对构造函数，析构函数，拷贝构造函数等的调用

##### 结果

```cpp
Test tst_arr[5] = {Test("1"), Test("2"), string("3"), "4"};
```

调用两次`Test(const char* str)`（其中包含两个`Mystr(const char* str)`调用）。一次`Test(string str)`，最后一次也是`Test(const char* str)`。这里的用法类似于隐式类型转换，若Test对象含有某个类型的构造函数，则相当于其定义了一个从该类型到Test的隐式类型转换（除非指定了explicit）

注意与书中不同，这里并没有调用拷贝构造，应该是编译器的优化

```cpp
void Test::setdata(string str)
{
#ifdef _TEST_DEBUG
    cout << "== Test setdata ==" << endl;
    cout << "pointer: " << this << endl;
#endif
    data = str;
}

Test changedata(Test tst)
{
    cout << "changedata call" << endl;
    tst.setdata(string("change"));
    cout << "setdata call end" << endl;
    return tst;
}

tst = changedata(tst);
```

编译器隐式传入了一个参数，该参数是一个临时的Test变量，且传入的参数被拷贝一次，因此实际上调用有点类似于

```cpp
Test tst;
Test tst_copy;
Test tst_tmp;

Test::Test(&tst_copy, &tst);		//这里代表拷贝构造函数
changedata(&tst_tmp, &tst_copy);	//返回值保存到tst_tmp中
```

不过比较令人匪夷所思的是这个tst_tmp没有再调用拷贝构造函数拷贝给tst，而是由编译器生成了一个浅拷贝函数赋值给了tst（这是逆向结果）

