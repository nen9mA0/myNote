## 本地

### commit

git commit

```
c1          c1
     --->    |
            c2
```

#### --amend

```
c1                 c1
 |   --->       __ | __
c2 - master*   c2      c2' - master*
```

### branch

git branch test

```
c1 - master*  --->    c1 -- master
                         |_ test*
```

git commit

```
c1 - master*  --->    c1 -- master
                       |
                      c2 -- test*
```

### merge

git merge test

```
c1            c3 - test         c1            c3 - test
 |                       --->   |              |     
c2 - master*                    c2             |
                                 |_____________|
                                         |
                                         c4 - master*
```

### rebase

将分支的提交线性合并到其他分支上

```
git rebase [目标分支]
git rebase [目标分支] [源分支]
```

git checkout test

git rebase master

```
c1            c3                c1            c3
|              |        --->    |             |         
c2 - master*  c4 - test         c2 - master   c4
                                |
                                c3'
                                |
                                c4' - test*
```

git rebase test master

```
c1            c3
|             |
c2            c4
|
c3'
|
c4' - test
    |_ master*
```

#### -i

默认情况下，合并时会按在test上的提交顺序合并，可以使用一个文档来编辑顺序与是否合并某些提交

### HEAD

HEAD默认指向当前branch指针指向的节点

#### 查看

```
cat .git/HEAD
git symbolic-ref HEAD
```

```
     c0
     |
|----c1-------|
|             |
c2 - master   c3
              |
              c4 - bugFix* - HEAD
```

git checkout c4

```
     c0
     |
|----c1-------|
|             |
c2 - master   c3
              |
              c4 - bugFix*
                 |__ HEAD
```

#### 相对引用

* ^ 向上引用一级父节点
* ^^ 向上引用两级父节点

```
     c0
     |
|----c1-------|
|             |
c2 - master   c3
              |
              c4 - bugFix* - HEAD
```

git checkout HEAD^^

```
     c0
     | /---------HEAD
|----c1-------|
|             |
c2 - master   c3
              |
              c4 - bugFix*
```

* ~n  向上引用n级父节点

git checkout HEAD~3

```
     c0 - HEAD
     |
|----c1-------|
|             |
c2 - master   c3
              |
              c4 - bugFix*
```

#### 多分支的相对引用

```
     c0
     |
|----c1-------|
|             |
c2            c3
|             |
|             c4
|_____________|
     |
     c5 - master*
```

git checkout HEAD^

```
     c0
     |
|----c1-------|
|             |
c2 - HEAD     c3
|             |
|             c4
|_____________|
     |
     c5 - master
```

git checkout HEAD^2

```
     c0
     |
|----c1-------|
|             |
c2            c3
|             |
|             c4 - HEAD
|_____________|
     |
     c5 - master
```

支持链式操作，还是一开始的例子

git checkout HEAD^2~1

```
     c0
     |
|----c1-------|
|             |
c2            c3 - HEAD
|             |
|             c4
|_____________|
     |
     c5 - master*
```



### reset

回退

```
     c0
     |
|----c1-------|
|             |
c2 - master   c3
              |
              c4 - bugFix*
```

git reset c3

```
     c0
     |
|----c1-------|
|             |
c2 - master   c3 - bugFix*
              |
              c4
```

### revert

以提交方式回退，常用于远程仓库，因为远程仓库无法reset

```
c0
|
c1
|
c2 - master*
```

git revert c2

```
c0
|
c1
|
c2
|
c2' - master*
```

### cherry-pick

选择特定的提交内容合并到当前branch。常用于有多个分支时却不想合并所有分支，可以将分支的最后一个提交单独取出放到目标分支

```
              c0
              | /----master*
|-------------c1------------|
|             |             |
c2            c4            c6
|             |             |
c3 - test1    c5 - test2    c7 - test3
```

git cherry-pick c3 c5 c7

```
              c0
              | /----master*
|-------------c1------------|------------|
|             |             |            |
c2            c4            c6          c3'
|             |             |            |
c3 - test1    c5 - test2    c7 - test3  c5'
                                         |
                                        c7' - master*
```

### tag

可以给某次提交建立一个锚点，永久标识这个提交

```
     c0 - HEAD
     |
|----c1-------|
|             |
c2 - master   c3
              |
              c4 - bugFix*
```

git tag t1 c3

```
     c0 - HEAD
     |
|----c1-------|
|             |
c2 - master   c3 <- t1
              |
              c4 - bugFix*
```

git checkout t1

```
     c0 - HEAD
     |
|----c1-------|
|             | /---HEAD
c2 - master   c3 <- t1
              |
              c4 - bugFix*
```

### describe

用于标识某个提交与距离最近的标签的位置

```
git discribe <ref>
```

若不指定ref则返回HEAD相对位置

```
返回格式： <tag>_<numCommits>_g<hash>
```

* tag 离ref最近的标签
* numCommits 相差几个commit
* hash ref的哈希值

若ref上本身有标签，则只会返回标签名

```
     c0 - HEAD
     |
|----c1-------|
|             |
c2 - master   c3 <- t1
              |
              c4 - bugFix*
```

git describe

```
t1_1_c4
```

## 远程

### clone

克隆一个仓库

克隆后本地仓库多了一个叫origin/master的分支，即远程仓库名（默认origin），远程仓库分支（这里是master）。注意这里与本地仓库的master是有区别的

#### 分离HEAD

因为orgin/master分支代表远程仓库，所以虽然可以checkout该分支，但不能直接在分支上提交（因为该分支应与远程同步）。此时若提交则HEAD仍会指向新的提交，但这里HEAD已经不指向origin/master了

```
c0
|
c1 - origin/master
   |_ master*
```

git checkout origin/master

```
c0
|
c1 - origin/master*
   |_ master
```

git commit

```
c0
|
c1 - origin/master
|  |_ master
c2 - HEAD   
```

### push

将本地仓库同步到远程仓库，同时修改origin/master指向的commit

```
本地                         远程
c0                           c0
|                            |
c1 - origin/master           c1 - master
|
c2 - master*
```

git push

```
本地                         远程
c0                           c0
|                            |
c1                           c1
|                            |
c2 - origin/master           c2 - master
   |_ master*
```

#### 参数

```
git push [远程仓库名] [分支名]
```

若当前HEAD没有指向某个分支或该分支没有指定远程跟踪，可以用该命令将指定分支push到远程仓库的同名分支，如

```
git push origin foo
```

结果

```
本地                 远程
c0  - foo    --->    c0  - foo
    |_ o/foo
```

##### 指定远程分支

```
git push origin [src]:[dst]
```

* src  **本地仓库**中的位置
* dst  **远程仓库**中的位置

```
git push origin foo:master
```

结果

```
本地                     远程
c0  - foo        --->    c0  - master
    |_ o/master
```

##### 指定某个提交

这里的foo^可以是任何一个commit

```
git push origin foo^:master
```

结果

```
本地                     远程
c0                       c0
|                        |
c1 - origin/master       c1 - master
|
c2 - foo
```

### fetch

下载远程仓库最新的提交，并同步到origin/master分支

注意这里**只有origin/master会被修改，master不会**

```
c0
|
c1 - origin/master
   |_ master*
```

若此时远程仓库有c2 c3两个新的提交

git fetch

```
c0
|
c1 - master*
|
c2
|
c3 - origin/master
```

#### 参数

```
git fetch [远程仓库名] [分支名]
```

仅fetch远程的指定分支

##### 指定本地分支或某个提交

```
git fetch [远程仓库] [src]:[dst]
```

- src  **远程仓库**分支/提交名
- dst  **本地仓库**分支名

这里参数前面是远程的后面是本地的，注意与push区分

### pull

同步完远程仓库后，一般需要将内容merge到本地仓库的master

`git pull`即为`git fetch`和`git merge`的合并指令

```
本地                         远程
c0                           c0
|                            |
c1 - origin/master           c1
|                            |
c2 - master*                 c3 - master
```

git pull

```
c0
|
c1____
|     |
c2    c3 - origin/master
|_____|
   |
   c4 - master
```

#### 参数

因为git pull是git fetch; git merge的合并，因此用法跟git fetch类似

```
本地                          远程
c0                            c0
|                      _______|_______
c1 - origin/bar        |             |
|  |_ origin/master    c2 - master   c3 - bar
c4 - master*
```

git pull origin bar:foo

```
       本地                          远程
       c0                            c0
       |                      _______|_______
       c1 - origin/bar        |             |
       |  |_ origin/master    c2 - master   c3 - bar
_______|_______
|              |
c4             c3 - foo
|______________|
       |
       c5 master*
```

git pull origin master:side

```
               本地                          远程
               c0                            c0
               |                      _______|_______
               c1 - origin/bar        |             |
               |  |_ origin/master    c2 - master   c3 - bar
_______________|_______________
|              |              |
c4             c3 - foo       c2 - side
|______________|              |
       |                      |
       c5 master*             |
       |______________________|
                   |
                   c6 - master*
```

### 远程提交与本地冲突

```
本地                         远程
c0                           c0
|                            |
c1 - origin/master           c1 - master
|                            |
c2 - master*                 c3
```

此时远程有一个新的提交c3

可以使用两种方法解决冲突

#### git pull

```
本地                                  远程
c0                                    c0
|                                     |
c1 ____________                       c1 - master
|              |                      |
c2 - master*   c3  - origin/master    c3
|______________|
       |
       c4
```

#### git pull --rebase

实际上是git fetch/git rebase，注意与单纯的git pull区别

```
本地                                  远程
c0                                    c0
|                                     |
c1 ____________                       c1 - master
|              |                      |
c2             c3  - origin/master    c3
               |
               c2' - master
```

### master不能提交的情况

有些项目不能直接提交到origin/master，而是应该先新建branch，并发起pull request

此时应该

* 将本地master reset为origin/master
* git branch -b 分支
* 修改自己代码
* git push

此时push上去自己的新分支

### 设置本地分支的远程跟踪

本地仓库一般通过origin/master与远程仓库关联。一般git默认设置master与origin/master关联，也可以指定一个本地分支与其关联

```
git checkout -b [本地] [远程]	切换分支并修改分支的远程跟踪对象
git branch -u [远程] [本地]		较普遍的做法，指定一个分支的远程跟踪
```

### 删除远程分支

push的一个特殊用法，当使用带参数的push而不指定src时会删除远程仓库的对应分支

```
git push origin :foo
```

效果是删除远程的foo分支和本地的origin/foo分支

fetch也有类似用法，不过效果是在本地创建一个分支，因此没有必要这样做