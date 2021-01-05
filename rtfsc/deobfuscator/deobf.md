## 数据结构

### array_list
作为一个容器，有点类似vector，支持的数据类型有int ptr string

```c
typedef struct ArrayList {
    void **al_elements;
    int al_size;
    int al_capacity;
    ATYPE al_type;
    int al_lock;
    int (*al_compare)(void *, void *);
    void (*al_print)(void *);
    void (*al_free)(void *);
} ArrayList;
```

主要定义了数据指针　当前大小　当前容量（默认10）　锁　还有三个成员函数（compare print free）

定义了三种type，分别是
* AL_LIST_UNSORTED
* AL_LIST_SORTED
* AL_LIST_SET

#### 创建
* al_new  创建一个unsorted类型的，存放ptr
* al_newT  类型自定义，存放ptr
* al_newInt  类型自定义，存放int
* al_newPtr  类型自定义，存放ptr
* al_newStr  类型自定义，存放string

al_newGeneric是上述几个函数调用的底层接口

#### 释放
* al_free　实际上内部只需要free掉al_elements，此后free整个结构体
* al_freeWithElements  带析构的释放，释放前调用结构体的al_free成员函数

#### 运算
* al_get  获取元素
* al_size  获取大小
* al_indexOf  获取与val相同元素的下标
* al_contains  是否包含val
* al_equals  两个array_list是否相同，对于set而言比较list2中是否有list1的每个元素，对于数组直接按下标一个个比较
* al_difference  list1和list2的差集
* al_intersection  list1和list2的交集（代码循环了两次，一次找list1中list2有的，一次找list2中list1有的，这找一次就好了吧，甚至如果type不是set的时候可能有bug）
* al_setEquals  比较set是否相等。先判断两者长度是否相等，再判断list1的每个元素是否都出现在list2，反之也判断一次（好像也有点脱裤子放屁，set中每个元素都是不同的，如果两个list相等且list1所有元素出现在list2那说明list2所有元素也都出现在list1，可能这人写代码保险起见喜欢这样干）
* al_union  list1和list2的并集

#### 添加
根据type定义了两种添加方式：al_insertSorted　al_insertAt
* al_insertAt  在某个index处添加元素，若为set则只会插入不一样的值
* al_insertSorted  按照顺序添加元素

此外留给用户的顶层接口为
* al_add  若为AL_LIST_SORTED则按顺序插入，否则在尾部插入
* al_addAll  在list1中使用al_add插入list2

#### 删除
* al_removeAt  删除index指示的元素
* al_remove  删除val指定的元素
* al_removeFirst
* al_removeLast

#### 排序
* al_sort

#### 对每个元素执行操作
* al_map  从第一个元素开始对每个函数执行func
* al_backmap  从最后一个元素开始对每个函数执行func

### arraylist 迭代器
```c
typedef struct Iterator {
    void *it_dataStructure;
    void *it_current;
    void *it_opaque1;
    void *it_opaque2;
    void *(*it_next)(struct Iterator *);
    int (*it_hasNext)(struct Iterator *);
    void *(*it_remove)(struct Iterator *);
} Iterator;
```

#### 创建
* al_iterator

#### next
* al_itrNext  迭代下一个元素
* al_itrHasNext  是否还有下一个元素
* al_itrRemove  删除迭代器当前返回的元素（注意迭代器指针在每轮迭代时总是指向下一个元素）

