# linux_kernel-list.h

看linux的代码看到这段利用头文件实现的链表。这段写的确实经典，简洁（但不算易懂）且用到了C语言的一些奇技淫巧（误），跟数据结构书上的比起来不知道要高到哪去（不过其实我也没有很注意看书上的代码就是了）

写了下注释

```C
    /*-
     * Copyright (c) 2011 Felix Fietkau <nbd@openwrt.org>
     * Copyright (c) 2010 Isilon Systems, Inc.
     * Copyright (c) 2010 iX Systems, Inc.
     * Copyright (c) 2010 Panasas, Inc.
     * All rights reserved.
     *
     * Redistribution and use in source and binary forms, with or without
     * modification, are permitted provided that the following conditions
     * are met:
     * 1. Redistributions of source code must retain the above copyright
     *    notice unmodified, this list of conditions, and the following
     *    disclaimer.
     * 2. Redistributions in binary form must reproduce the above copyright
     *    notice, this list of conditions and the following disclaimer in the
     *    documentation and/or other materials provided with the distribution.
     *
     * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
     * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
     * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
     * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
     * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
     * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
     * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
     * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
     * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
     * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
     */
    #ifndef _LINUX_LIST_H_
    #define _LINUX_LIST_H_

    #include <stddef.h>
    #include <stdbool.h>

    #define    prefetch(x)

        /*很有趣的一个实现
        __typeof__(((type *) NULL)->member) *__mptr 把mptr初始化为任意结构体的member元素对应类型的指针
        ptr为结构体内某元素的指针
        offsetof(type, member) 返回type结构体的member成员相对type的偏移量
        因此该宏定义作用是传入一个结构体实例的成员指针，返回该结构体的头指针
        */
    #ifndef container_of
    #define container_of(ptr, type, member)                    \
        ({                                \
            const __typeof__(((type *) NULL)->member) *__mptr = (ptr);    \
            (type *) ((char *) __mptr - offsetof(type, member));    \
        })
    #endif

    struct list_head {
        struct list_head *next;
        struct list_head *prev;
    };

    #define LIST_HEAD_INIT(name) { &(name), &(name) }    
            //这里用name的地址初始化prev和next，使name声明后本身是个回环
    #undef LIST_HEAD
    #define LIST_HEAD(name)    struct list_head name = LIST_HEAD_INIT(name)

    static inline void
    INIT_LIST_HEAD(struct list_head *list)
    {
        list->next = list->prev = list;
    }

    static inline bool
    list_empty(const struct list_head *head)
    {
        return (head->next == head);
    }

    static inline bool
    list_is_first(const struct list_head *list,
              const struct list_head *head)
    {
        return list->prev == head;            //如果前一个元素就是链表头
    }

    static inline bool
    list_is_last(const struct list_head *list,
             const struct list_head *head)
    {
        return list->next == head;            //如果后一个元素是链表尾
    }

    static inline void
    _list_del(struct list_head *entry)
    {
        entry->next->prev = entry->prev;
        entry->prev->next = entry->next;
    }

    static inline void
    list_del(struct list_head *entry)
    {
        _list_del(entry);
        entry->next = entry->prev = NULL;
    }                    
            //这里没有free操作，说明这些函数不处理内存分配问题

    static inline void
    _list_add(struct list_head *_new, struct list_head *prev,
        struct list_head *next)
    {

        next->prev = _new;
        _new->next = next;
        _new->prev = prev;
        prev->next = _new;
    }                        //同样没有malloc

    static inline void
    list_del_init(struct list_head *entry)
    {
        _list_del(entry);
        INIT_LIST_HEAD(entry);
    }
            /*假设list_head是一个容器（结构体）内的成员
            通过list_entry获取该容器对应实例的头指针
            ptr为指向list_head的指针*/
    #define    list_entry(ptr, type, field)    container_of(ptr, type, field)
    #define    list_first_entry(ptr, type, field)    list_entry((ptr)->next, type, field)
    #define    list_last_entry(ptr, type, field)    list_entry((ptr)->prev, type, field)

    #define    list_for_each(p, head)                        \
        for (p = (head)->next; p != (head); p = p->next)

    #define    list_for_each_safe(p, n, head)                    \
        for (p = (head)->next, n = p->next; p != (head); p = n, n = p->next)

    #define list_for_each_entry(p, h, field)                \
        for (p = list_first_entry(h, __typeof__(*p), field); &p->field != (h); \
            p = list_entry(p->field.next, __typeof__(*p), field))
                                            //h为链表头

    #define list_for_each_entry_safe(p, n, h, field)            \
        for (p = list_first_entry(h, __typeof__(*p), field),        \
            n = list_entry(p->field.next, __typeof__(*p), field); &p->field != (h);\
            p = n, n = list_entry(n->field.next, __typeof__(*n), field))

    #define    list_for_each_entry_reverse(p, h, field)            \
        for (p = list_last_entry(h, __typeof__(*p), field); &p->field != (h); \
            p = list_entry(p->field.prev, __typeof__(*p), field))

    #define    list_for_each_prev(p, h) for (p = (h)->prev; p != (h); p = p->prev)
    #define    list_for_each_prev_safe(p, n, h) for (p = (h)->prev, n = p->prev; p != (h); p = n, n = p->prev)

    static inline void
    list_add(struct list_head *_new, struct list_head *head)
    {
        _list_add(_new, head, head->next);
                //这里的第三个参数结合list_init构造了一个循环链表
    }

    static inline void
    list_add_tail(struct list_head *_new, struct list_head *head)
    {
        _list_add(_new, head->prev, head);
    }

    static inline void
    list_move(struct list_head *list, struct list_head *head)
    {
        _list_del(list);
        list_add(list, head);
    }

    static inline void
    list_move_tail(struct list_head *entry, struct list_head *head)
    {
        _list_del(entry);
        list_add_tail(entry, head);
    }

    static inline void
    _list_splice(const struct list_head *list, struct list_head *prev,
        struct list_head *next)
    {
        struct list_head *first;
        struct list_head *last;

        if (list_empty(list))
            return;

        first = list->next;
        last = list->prev;
        first->prev = prev;
        prev->next = first;
        last->next = next;
        next->prev = last;
    }    
            //将头为prev尾为next（其实头尾一样）的链表拼接到list链表内，注意拼接后list不再是链表头

    static inline void
    list_splice(const struct list_head *list, struct list_head *head)
    {
        _list_splice(list, head, head->next);
    }
                                            //list为待插入链表
    static inline void
    list_splice_tail(struct list_head *list, struct list_head *head)
    {
        _list_splice(list, head->prev, head);
    }

    static inline void
    list_splice_init(struct list_head *list, struct list_head *head)
    {
        _list_splice(list, head, head->next);
        INIT_LIST_HEAD(list);
    }

    static inline void
    list_splice_tail_init(struct list_head *list, struct list_head *head)
    {
        _list_splice(list, head->prev, head);
        INIT_LIST_HEAD(list);
    }
                        //插入list后将list头指针初始化
    #endif /* _LINUX_LIST_H_ */
```

## 函数功能总结

### 结构体

list_head 双向循环链表

### 宏

container_of( 指针,成员类型,成员名 )    传入结构体某个成员的指针返回结构体头指针

### 函数

INIT_LIST_HEAD    初始化链表

list_empty    判断列表是否为空

list_is_first    判断当前元素是否为第一个

list_is_last    判断当前元素是否为最后一个

===================================

_list_del    删除元素通用实现

list_del    删除head指向元素

_list_add    添加元素通用实现

list_add    头插法，在链表头添加一个元素

list_add_tail    尾插法，链表尾添加一个元素

list_del_init    删除并初始化

list_move    移动list元素到链表头

list_move_tail    移动list到链表尾

===================================

list_entry    获取list_head所在容器的头指针

list_first_entry    上一个容器的头指针

list_last_entry    下一个容器的头指针

===================================

list_for_each

list_for_each_safe    向后遍历链表

list_for_each_prev

list_for_each_prev_safe    向前遍历链表

list_for_each_entry

list_for_each_entry_safe    遍历容器中的链表，p指向容器头

list_for_each_entry_reverse    反向遍历

===================================

_list_splice    链表拼接通用实现

list_splice    头插法将list插入head链表

list_splice_tail    尾插法

list_splice_init

list_splice_tail_init    插入后初始化

===================================

**一个对于CONTAINER_OF和LIST_ENTRY解释得不错的链接**

<https://blog.csdn.net/hs794502825/article/details/10364977>

## 几个比较有意思的宏

# offsetof

出处：openwrt/libubox/list.h

```C
#define offsetof(s,m)  (size_t)&(((s *)0)->m)
```

这个宏的作用是求结构体的一个成员相对结构体首地址的偏移量

```C
((s *)0)->m
```

跟

```C
((s *)NULL)->m
```

等价，这里做了件事，就是将数字0（NULL）强制转换成一个地址，使它的类型为指向结构体的指针s*
因此如果调用 offsetof(A,pointer)实际上调用了

```C
(size_t) &((A *)NULL)->pointer
```

假设pointer成员相对A类型头指针的偏移量为n，那么上述运算的结果即为n
因为 &((A*)NULL) 值为0，所以 &(((A *)NULL)->pointer)值为n
这里注意符号&，原因我就不多说了

# container_of

出处：openwrt/libubox/list.h

```C
#define container_of(ptr, type, member)                    \
({                                \
    const __typeof__(((type *) NULL)->member) *__mptr = (ptr);    \
    (type *) ((char *) __mptr - offsetof(type, member));    \
})
```

这个宏用处在于假设定义了一个结构体

```C
typedef struct mystruct{
    int num;
    list_head pointer;
} A;
```

这里list_head是双向链表的指针，定义如下

```C
typedef struct _list_head{
    struct *list_head prev;
    struct *list_head next;
} list_head;
```

定义

```C
A a;
int *p = (int*) &a.pointer.next;    //这里我强制转换成(int*)是省事而已，其实是个不好的做法
```

此时调用

```C
void *p = NULL;
p = container_of(p,A,pointer);
```

会得到指向a的指针，原因如下，把宏展开

```C
p = ({ const __typeof__( ( (A *)NULL ) ->pointer) *mptr = (p);
       (A*) ( (char*) __mptr - offsetof(A,pointer) ) })
```

还是有点难懂，拆分一下
第一个问题是，mptr的类型究竟是什么，答案是

```C
const __typeof__( ( (A *)NULL ) ->pointer)
```

再拆分，__typeof__( ( (A *)NULL ) ->pointer)是什么？，其实就是A类型的pointer这个成员的类型，即list_head
__typeof__是gnu C编译器的一个宏，返回变量的类型
所以mptr的类型是 const list_head *
所以这个宏的第一步是

```C
const list_head * mptr = (p);
```

再看第二步

```C
(A*) ( (char*) __mptr - offsetof(A,pointer) ) 
```

第一步定义的mptr减去mptr指向的成员（在本例中是pointer）相对A结构体首地址的偏移
因此得到的就是该A结构体实例的首地址，即a，这也是为什么第二步的最外层是个(A*)的强制转换
