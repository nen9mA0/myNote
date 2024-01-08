[GitHub - rpoxo/bf2mesh: A Battlefield 2 mesh file parser written in Python](https://github.com/rpoxo/bf2mesh/)

## 类

### bf2head

文件头，在mesh的文件起始  20B

* u1  4B  可能是bfp4f的版本号

* version  4B  版本

* u3  4B

* u4  4B

* u5  4B

### BF2Mesh

VisibleMesh的基类，主要用于判断mesh文件的类型

* isSkinnedMesh  后缀为`.skinnedmesh`

* isBundledMesh  后缀为`.bundledmesh`

* isStaticMesh  后缀为`.staticmesh`

* isCollisionMesh  后缀为`.collisionmesh`

### bf2lod

记录LOD信息，包含两类

* node/rig信息
  
  * 对于bundledmesh是node信息
  
  * 对于skinnedmesh是rig信息
  
  * 对于staticmesh是boundary信息

* 材质（material）信息，是[bf2mat](#bf2mat)结构列表

#### load_nodes_rigs

node/rig信息读取

* 读取float3  min属性

* 读取float3  max属性

* 若为低版本（4/6），读取float3  pivot属性

* 若为SkinnedMesh，读取rig信息
  
  * 读取4B  rignum
  
  * 读取rignum个[bf2rig](#bf2rig)结构体

* 否则为BundledMesh或StaticMesh，读取node信息
  
  * 读取4B  nodenum
  
  * 若为StaticMesh
    
    * 读取nodenum个matrix4结构（4*4个float）

#### load_materials

材质信息读取

* 读取4B  matnum

* 读取matnum个[bf2mat](#bf2mat)结构体

### bf2geom

几何结构表（Geometry structure table），用于记录lod信息

* lodnum  lod元素个数

* lods  [bf2lod](#bf2lod)结构体列表

### bf2vertattrib

顶点属性表，记录顶点数据是如何被记录于vertices数组的

* flag  2B  是否被使用

* offset  2B  相对块起始的偏移

* vartype  2B  D3DDECLTYPE结构体

* usage  2B  D3DDECLUSAGE结构体

### bf2mat

* alphamode  4B  是否使用透明度属性
  
  若为SkinnedMesh则不存在该字段

* fxfile  字符串  shader路径

* technique  字符串  透明度类型

* mapnum  4B  材质个数

* maps  mapnum个材质路径字符串

* vstart  4B  顶点数组偏移

* istart  4B  索引数组偏移

* inum  4B  索引数

* vnum  4B  顶点数

* u4  4B

* u5  4B

* 若不是SkinnedMesh且version为11
  
  * mmin  float3  材质边界 min
  
  * mmax  float3  材质边界 max

### bf2rig

* bonenum  4B  bone结构个数

* bones  bf2bone结构列表

### bf2bone

* id  4B

* matrix  matrix4，即4*4个float

## 文件加载过程

* 读取bf2head结构

* 读取1字节u1

* 读取geom结构，该结构用于存储材质信息
  
  staticmesh: geom0 = non-destroayble, geom1 = 3p destroyed
  
  skinnedmesh: geom0 = 1p, geom1 = 3p
  
  bundledmesh: geom0 = 1p, geom1 = 3p, geom2 = 3p wreck
  
  * 读取4字节geom_num
  
  * 读取geom_table（geom_num个[bf2geom](#bf2geom)结构体）

* 读取顶点属性表，该结构表示了关于顶点信息如何被存储的信息
  
  * 读取4字节vertattrib_num
  
  * 读取vertattrib_table（vertattrib_num个[bf2vertattrib](#bf2vertattrib)结构体）
  
  * 读取4字节vert_format，该属性表示了每个顶点数组元素的长度，目前都是4
  
  * 读取4字节vert_stride，该属性表示了每个顶点数据块的长度

* 读取顶点数据
  
  * 读取4字节vert_num
  
  * 读取顶点数组vertices，每个元素均为float。读取元素的个数为 `vert_stride/vert_format * vert_num`，即每个数据块长度除以每个元素长度乘顶点元素数

* 读取索引数据
  
  * 读取4字节索引数index_num
  
  * 读取索引数组indices

* 若为SkinnedMesh，读取4字节u2

* 读取geom_table中记录的具体LOD信息，对geom_table中的每个geom循环
  
  * 对geom中的每个LOD循环
  
  * 调用[load_nodes_rigs](#load_nodes_rigs)读取rig/node信息

* 与上面相同，进行循环，差别只是调用[load_materials](#load_materials)读取材质信息
