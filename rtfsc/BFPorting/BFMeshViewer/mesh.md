### 一些资料

[Understanding Meshes | Classic Battlefield Modding Wikia | Fandom](https://classic-battlefield-modding.fandom.com/wiki/Understanding_Meshes)  BF2 mesh解释，主要关于LOD

https://blog.kokojia.com/game/b-53.html  UV坐标

[什么是3D中的顶点缓冲与索引缓冲_游戏开发者博客_课课家博客](https://blog.kokojia.com/game/b-54.html)  顶点缓冲/索引缓冲

[面法线与顶点法线_游戏开发者博客_课课家博客](https://blog.kokojia.com/game/b-56.html)  顶点法线

## 代码解析

Mesh文件格式及加载方式

### 文件格式

在modVisMeshStruct下

```visual-basic
'file structure
Public Type bf2mesh

    'header
    head As bf2head

    'unknown
    u1 As Byte 'always 0?

    'geoms
    geomnum As Long
    geom() As bf2geom

    'vertex attribute table
    vertattribnum As Long
    vertattrib() As bf2vertattrib

    'vertices
    vertformat As Long 'always 4?  (e.g. GL_FLOAT)
    vertstride As Long
    vertnum As Long
    vert() As Single

    'indices
    indexnum As Long
    index() As Integer

    'unknown
    u2 As Long 'always 8?

    ''''internal
    filename As String       'current loaded mesh file
    fileext As String        'filename extension
    isStaticMesh As Boolean  'true if file extension is "staticmesh"
    isSkinnedMesh As Boolean 'true if file extension is "skinnedmesh"
    isBundledMesh As Boolean 'true if file extension is "bundledmesh"
    isBFP4F As Boolean       'true if file is inside BFP4F directory
    loadok As Boolean        'mesh loaded properly
    drawok As Boolean        'mesh rendered properly
    stride As Long           'vertstride / 4
    facenum As Long          'facenum / 3
    polynum As Long          'number of polygons
    elemnum As Long          'number of elements
    normoff As Long          'vertex array normal vector offset
    tangoff As Long          'vertex array tangent vector offset
    texcoff As Long          'vertex array UV0 offset
    xtan() As float4         'corrected tangents
    uvnum As Long            'number of detected uv channels
    editinfo As Boolean      'has mesh_edit info computed

    vertinfo() As fh2vert    'vertex info & flags
    faceinfo() As fh2face    'face info & flags
    polyinfo() As fh2poly    'polygon info & flags
    eleminfo() As fh2elem    'element info & flags

    hasSkinVerts As Boolean  'deformed vertices flag
    skinvert() As float3     'deformed vertices
    skinnorm() As float3     'deformed normals
End Type
```

一些重要成员如下：

* head  文件头，一些信息

* LOD信息
  
  * geomnum  geom数组元素个数
  
  * geom  数组，包含了LOD信息，见[LOD](#LOD)

* 顶点属性表信息
  
  * vertattribnum  vertattrib数组元素个数
  
  * vertattrib  数组，顶点属性数据

* 顶点
  
  * vertformat  顶点格式
  
  * vertstride
  
  * vertnum  顶点数
  
  * vert  数组，顶点数据

* 索引 暂时不知道是什么
  
  * indexnum 索引个数
  
  * index 数组，索引数据

下面的属性与文件本身无关，是自定义的缓存数据
