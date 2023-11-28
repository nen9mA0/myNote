### mesh重建

在scene_manipulator的import_mesh函数

#### 多边形重建

bf2mesh结构中的geoms成员保存了多个bf2geom结构，每个结构中包含多个bf2lod结构。每个bf2lod都定义了一个新的模型。每个bf2lod结构体中都有一个bf2mat列表，根据bf2mat列表的内容可以得到顶点列表的起始和结束下标

##### 顶点与顶点索引

* 顶点数组为bf2mesh的vertices列表，以bf2mat结构的vstart为起始下标，vstart+vnum为结束下标，每三个值构成一个顶点的x y z坐标

* 顶点索引数组为bf2mesh的index列表，以bf2mat结构的istart为起始下标，istart+inum为结束下标，每三个值构成一个三角形面元的顶点索引

##### 顶点属性

bf2mesh结构中的vertex_attributes属性，列表中每个元素都是一个bf2vertattrib类。usage记录了该类的用途，offset定义了对应属性的存放在vertices数组的偏移

分别有下列属性（即usage的取值）

* 2  weight

* 3  法线

* 6  切线

* UV通道，有多个取值
  
  - 通道0 5
  
  - 通道1 261
  
  - 通道2 517
  
  - 通道3 733
  
  - 通道4 1029

属性对应的数据都存放在对应bf2mat结构的 `bf2mat.vstart + bf2vertattrib.offset` 位置处

###### 法线

法线的x y z为对应三角形面元的vertices下标加上偏移

###### UV

UV的U通道和V通道为对应三角形面元的vertices下标加上偏移
