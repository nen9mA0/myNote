### mesh

* bf2head  20B  文件头
  
  - u1 4B 可能是bfp4f的版本号
  
  - version 4B 版本
  
  - u3 4B
  
  - u4 4B
  
  - u5 4B

* u1  1B

* geom_num  4B  geom结构个数
  
  每个bf2geom结构都包含多个bf2lod结构，文件接下来记录了每个bf2geom结构包含多少个bf2lod
  
  * lodnum  4B  bf2lod结构个数，共有geom_num条记录

* vertattrib_num  4B  顶点属性表的个数

* 下面是顶点属性表，共有vertattrib_num个，每个顶点属性表内容如下
  
  - flag 2B 是否被使用
  
  - offset 2B 相对块起始的偏移
  
  - vartype 2B D3DDECLTYPE结构体
  
  - usage 2B D3DDECLUSAGE结构体
- vert_format  4B  每个顶点数组元素的长度，目前内容都是4

- vert_stride  4B  每个顶点数据块的长度

- vert_num  4B  顶点数

- vertices  顶点数组  每个元素均为float。读取元素的个数为 `vert_stride/vert_format * vert_num`，即每个数据块长度除以每个元素长度乘顶点元素数

- index_num  4B  索引数组元素个数

- indices  索引数组  index_num个元素，每个元素都为2B

- 若为SkinnedMesh，读取4字节u2 

- bf2geom中记录的具体LOD信息，共有geom_num*lodnum个
  
  - min  float3  12B
  
  - max  float3  12B
  
  - 若为低版本（4/6，bf2head中的version属性），为pivot属性  float3
  
  - 若为SkinnedMesh，为rig信息
    
    - rignum  4B  rig结构数
    
    - rignum个bf2rig结构
      
      - bonenum 4B bone结构个数
      
      - bonenum个bf2bone结构
        
        - id 4B
        
        - matrix matrix4，即4*4个float
  * 否则为BundledMesh或StaticMesh，读取node信息
    
    - 读取4B nodenum
    
    - 若为StaticMesh
      
      - 读取nodenum个matrix4结构（4*4个float）

- bf2lod中的材质信息，也有geom_num*lodnum个
  
  - matnum  4B  bf2mat结构体个数
  
  - 下面是matnum个bf2mat结构，每个结构内容如下
    
    - 若不是SkinnedMesh，为alphamode 4B 是否使用透明度属性
    
    - fxfile 字符串 shader路径
    
    - technique 字符串 透明度类型
    
    - mapnum 4B 材质个数
    
    - maps mapnum个材质路径字符串
    
    - vstart 4B 顶点数组偏移
    
    - istart 4B 索引数组偏移
    
    - inum 4B 索引数
    
    - vnum 4B 顶点数
    
    - u4 4B
    
    - u5 4B
    
    - 若不是SkinnedMesh且version为11
      
      - mmin float3 材质边界 min
      
      - mmax float3 材质边界 max

### Skeleton

骨架文件

* version  4B  版本号

* node_num  4B  节点数

* 循环遍历node_num个节点，每个节点包含下列元素
  
  * 节点名表
    
    * node_name_char_count  2B  节点名长度
    
    * node_name  nB  节点名
  
  * node_parent_index  2B  父节点索引
  
  * 节点四元数旋转矩阵
    
    * x  4B float
    
    * y  4B float
    
    * z  4B float
    
    * w  4B float
  
  * 节点位置
    
    * x  4B float
    
    * y  4B float
    
    * z  4B float

若没有父节点，则根据节点名判断节点的类型。若名为`Camerabone`则为相机，否则为根节点

### Animation

动画文件

* version 4B 版本号

* bone_num  2B  骨骼数

* 每个bone对应的索引，循环bone_num次，每次读取一个索引号
  
  * bone_id  2B  骨骼索引

* frame_num  4B  帧数

* precision  1B  精度

* 每个bone的帧数据
  
  * data_size  2B  像是当前bone的帧数据长度，但在程序里没被使用
  
  * 接下来是帧数据块，帧数据块的组织形式如下面实例所示。首先帧数据分为7个类别顺序存储，分别是当前bone的旋转四元数x y z w分量和其位置分量x y z
    
    此外，为了方便数据压缩，同类别的帧数据被划分为多个帧数据块，每个帧数据块包含多个帧。在同一个帧数据块内可能用到rle压缩方法，这种压缩就是把连续帧中数据相同的压缩到同一个块中
    
    * data_left  2B  记录了当前多个帧数据块的总大小
    
    * 接下来是每个帧数据块的内容
      
      * head  1B  帧头，其中最高位表示该帧是否被压缩，低7位表示帧的数量
      
      * next_header  1B  下一个帧头的与该帧的偏移
      
      * 若该帧被压缩，则读取一个2B的value作为该帧块所有帧的数据。否则使用一个循环每次读取2B的value

```
bone_1
    data_size  2B
        第一类帧数据，这里value作为帧块1的四元数x分量
        data_left  2B
            |--- frame block 1 ---|
            |  head  1B
            |  next_header  1B
            |  value  若有压缩，则只有2B，若无压缩，则有帧数*2B
            |--- frame block 2 ---|
            |  head  1B
            |  next_header  1B
            |  value  若有压缩，则只有2B，若无压缩，则有帧数*2B
            ......
        第二类帧数据，这里value作为帧块1的四元数y分量
        data_left  2B
            |--- frame block 1 ---|
            |  head  1B
            |  next_header  1B
            |  value  若有压缩，则只有2B，若无压缩，则有帧数*2B
            |--- frame block 2 ---|
            |  head  1B
            |  next_header  1B
            |  value  若有压缩，则只有2B，若无压缩，则有帧数*2B
            ......
        ......
bone_2
    data_size  2B
        第一类帧数据，这里value作为帧块1的四元数x分量
        data_left  2B
            |--- frame block 1 ---|
            |  head  1B
            |  next_header  1B
            |  value  若有压缩，则只有2B，若无压缩，则有帧数*2B
            |--- frame block 2 ---|
            |  head  1B
            |  next_header  1B
            |  value  若有压缩，则只有2B，若无压缩，则有帧数*2B
            ......
        第二类帧数据，这里value作为帧块1的四元数y分量
        data_left  2B
            |--- frame block 1 ---|
            |  head  1B
            |  next_header  1B
            |  value  若有压缩，则只有2B，若无压缩，则有帧数*2B
            |--- frame block 2 ---|
            |  head  1B
            |  next_header  1B
            |  value  若有压缩，则只有2B，若无压缩，则有帧数*2B
            ......
        ......
```
