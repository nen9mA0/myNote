## 简介

文件在glibc下的misc/sys/queue.h

绝大多数功能都是通过宏实现的

## 数据结构

### LIST

实现了一个双链表结构

#### 链表定义

**注意：这里链表元素中，le_next为指针，而le_prev为双重指针，保存的是前一个元素的le_next元素的地址**

```c
#define    LIST_HEAD(name, type)                        \
struct name {                                \
    struct type *lh_first;    /* first element */            \
}

#define    LIST_HEAD_INITIALIZER(head)                    \
    { NULL }

#define    LIST_ENTRY(type)                        \
struct {                                \
    struct type *le_next;    /* next element */            \
    struct type **le_prev;    /* address of previous next element */    \
}

/*
 * List functions.
 */
#define    LIST_INIT(head) do {                        \
    (head)->lh_first = NULL;                    \
} while (/*CONSTCOND*/0)
```

* LIST_HEAD  定义双链表头节点

* LIST_ENTRY 定义双链表元素

* LIST_HEAD_INITIALIZER  初始化链表头节点

* LIST_INIT  也是初始化链表头节点

#### 链表操作

```c
#define    LIST_INSERT_AFTER(listelm, elm, field) do {            \
    if (((elm)->field.le_next = (listelm)->field.le_next) != NULL)    \
        (listelm)->field.le_next->field.le_prev =        \
            &(elm)->field.le_next;                \
    (listelm)->field.le_next = (elm);                \
    (elm)->field.le_prev = &(listelm)->field.le_next;        \
} while (/*CONSTCOND*/0)

#define    LIST_INSERT_BEFORE(listelm, elm, field) do {            \
    (elm)->field.le_prev = (listelm)->field.le_prev;        \
    (elm)->field.le_next = (listelm);                \
    *(listelm)->field.le_prev = (elm);                \
    (listelm)->field.le_prev = &(elm)->field.le_next;        \
} while (/*CONSTCOND*/0)

#define    LIST_INSERT_HEAD(head, elm, field) do {                \
    if (((elm)->field.le_next = (head)->lh_first) != NULL)        \
        (head)->lh_first->field.le_prev = &(elm)->field.le_next;\
    (head)->lh_first = (elm);                    \
    (elm)->field.le_prev = &(head)->lh_first;            \
} while (/*CONSTCOND*/0)

#define    LIST_REMOVE(elm, field) do {                    \
    if ((elm)->field.le_next != NULL)                \
        (elm)->field.le_next->field.le_prev =             \
            (elm)->field.le_prev;                \
    *(elm)->field.le_prev = (elm)->field.le_next;            \
} while (/*CONSTCOND*/0)

#define    LIST_FOREACH(var, head, field)                    \
    for ((var) = ((head)->lh_first);                \
        (var);                            \
        (var) = ((var)->field.le_next))
```

* LIST_INSERT_AFTER  在双链表某个元素listelm后插入一个元素elm
  
  * `elm->next=listelm->next`，若不为NULL则`listelm->next->prev=elm`
  
  * `listelm->next=elm`
  
  * `elm->prev=listelm`

* LIST_INSERT_BEFORE  在双链表某个元素listelm前插入一个元素elm

* LIST_INSERT_HEAD  在双链表头插入一个元素elm
