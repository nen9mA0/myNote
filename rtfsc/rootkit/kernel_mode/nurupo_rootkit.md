与ivyl rootkit采用的原理大体相同，但有一些处理更加完善

#### 模块保护和隐藏

模块隐藏与ivyl基本相同，但没有对sysfs的处理

加上了使用 `try_module_get(THIS_MODULE)` 增加模块引用计数防止其被删除的方式

#### proc fops hook

nurupo实现了几个hook，还有根据版本的处理（这里代码只写了4.4.0~4.5.0和2.6.32两种情况，不知道是不是他只测试了这几个版本）

* `/  /proc  /sys` 的readdir（2.x内核）或iterate（4.x内核）的hook

* 对rootkit创建的proc文件的read、write的hook

前一个hook与ivyl基本相同

对于第二个hook，这里同样先拿到 `/proc` 的proc_dir_entry结构，再遍历subdir成员指向的内容。这里不同内核需要不同的遍历方式：4.x的内核采用红黑树，2.x的则是应该简单的链表

拿到后门对应文件的fops后，hook其write和read函数
