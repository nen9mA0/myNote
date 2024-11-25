### 解包

https://github.com/extremecoders-re/pyinstxtractor/blob/master/pyinstxtractor.py

#### 文件头

* pyinstaller使用一个魔数作为文件起始的标志，魔数为`MEI\014\013\012\013\016`

* 魔数后是pyinstaller的其他信息
  
  * 若为2.0，信息长度为16，加上魔数长度为24，分别有下列字段
    
    * 包长度，int
    
    * toc，内容目录（Table of Content），int
    
    * toc长度，int
    
    * python版本，int
  
  * 若为2.1+，信息长度为16+64，加上魔数长度为24+64，且从第24个字符之后为字符串`python`
    
    - 包长度，uint
    
    - toc，内容目录（Table of Content），uint
    
    - toc长度，int
    
    - python版本，int
    
    - python库名称，64字节字符串

#### TOC

TOC放在包的末尾（注意不是二进制文件的末尾，二进制文件末尾还有ELF结构/PE结构的其他段）

TOC由一系列TOC Entry组成每个TOC Entry格式如下，注意，该结构为**大端**

* Entry长度，大端int

* Entry地址，大端uint

* 压缩数据长度，大端uint

* 解压后数据长度，大端uint

* 压缩标志，uchar

* 压缩数据类型，char

* 文件名，字符串

#### 代码压缩文件

每个TOC Entry都定义了一个代码压缩文件

* 位置就在Entry地址字段，长度由压缩数据长度字段定义

* 若压缩标志为1，则表示为zip压缩的文件，使用zlib解压并比较解压后长度与TOC Entry的解压后数据长度字段

* 判断压缩数据类型
  
  * 若为d或o（ARCHIVE_ITEM_DEPENDENCY或ARCHIVE_ITEM_RUNTIME_OPTION），不进行任何操作
  
  * 若为s（ARCHIVE_ITEM_PYSOURCE），则为pyc数据，格式化为pyc文件后写入对应`.pyc`
  
  * 若为M或m（ARCHIVE_ITEM_PYPACKAGE或ARCHIVE_ITEM_PYMODULE），则为模块的pyc数据，简单判断文件头后写入对应`.pyc`
  
  * 其他情况直接输出为Entry指定的文件名，其中若压缩数据类型为z或Z说明是pyz文件，会解析对应pyz文件


