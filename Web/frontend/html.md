教程

[使用 HTML 组织网站内容 - 学习 Web 开发 | MDN](https://developer.mozilla.org/zh-CN/docs/Learn/HTML)

## HTML元素

### 简介

![](pic/1.png)

一个元素（element）包含三个部分

* 开始标签

* 结束标签

* 内容

元素可以被嵌套（即在一个元素中包含其他元素）

### 分类

#### 块级元素

一般用于展示结构化的内容，如段落、列表、导航菜单、页脚。

它会出现在新的一行，其后的内容也会被挤到下一行展现

不能被嵌套在内联元素中

如 `<p>`

#### 内联元素

通常出现在块级元素中并环绕文档内容的一小部分

内联元素不会导致文本换行：它通常出现在一堆文字之间

如 `<a>`

#### 空元素

通常只有一个标签，用来在此位置插入一些内容

如 `<img>`

### 属性

![](pic/2.png)

这里需要注意的是布尔属性，如 `<input>` 标签

下面两种形式等价

```html
<input type="text" disabled="disabled">

<input type="text" disabled>
```

### 标签

#### 文档相关

##### <html>

用于标记整个html文档的标签，可以指定一些属性

* lang  网页语言

###### <head>

不会显示，用于保存页面的一些**元数据**，一般包含

###### <title>

网站标题

###### <meta>

可以是任意添加的元数据，常见的有

* charset  字符集
  
  ```html
  <meta charset="utf-8">
  ```

* name  content
  
  提供类似键值对的存储
  
  ```html
  <meta name="author" content="Chris Mills">
  ```

* link  用于添加外部链接，根据`rel`属性可以有很多用途，如
  
  * 图标，rel为icon
    
    ```html
    <link rel="icon" href="favicon.ico" type="image/x-icon">
    ```
  
  * 样式表，rel为stylesheet
    
    ```html
    <link rel="stylesheet" href="my-css-file.css">
    ```

* script  引用的js脚本

此外每个网站都可能有自定义的元数据类型和协议，用于提供一系列信息

#### 结构相关

##### 页眉

`<header>`

##### 导航栏

`<navi>`

##### 主内容

* `<main>`  一般一个文档里只包含一个

* `<article>`  一般用于包裹一篇文章的内容

* `<section>`  用于包含不同的段落

##### 侧边栏

`<aside>`

##### 页脚

`<footer>`

##### 无语义元素

一种比较特殊的元素，当你需要增加一个标签但没有合适的标签来包含内容时使用

* `<div>` 块级

* `<span>`  行内

#### 文字内容相关

##### 标题

`<h1>` 到 `<h6>`

##### 段落

`<p>`

##### 列表

###### 无序列表

无序列表使用`<ul>`（unordered list）声明，并用`<li>`（list item）包裹每个元素

###### 有序列表

有序列表使用`<ol>`（ordered list）声明，也用`<li>`（list item）包裹每个元素

###### 嵌套列表

上述列表间可以嵌套构成多级列表

##### 超链接

`<a>` 可以通过title属性设置显示的标题

###### 指向id的超链接

此外，可以通过设置元素的`id`属性，配合URL的`#`来完成对某个页面某个特定元素的定位

如定义

```html
<h2 id="Mailing_address">邮寄地址</h2>
```

可以通过

```html
<a href="contacts.html#Mailing_address">
```

定位

###### 电子邮件

通过mailto关键字，如

```html
<a href="mailto:nen9ma0@outlook.com">Send Mail</a>
```

##### 字体效果

###### 强调

`<em>`，显示出的是斜体

###### 着重强调

`<strong>`，显示出的是粗体

###### 粗体 斜体 下划线

* `<i>` 斜体

* `<b>` 粗体

* `<u>` 下划线

##### 换行 分割线

* `<br>`  换行

* `<hr>`  分割线

##### 描述列表

###### Description List

`<dl>` 用于声明一个描述列表，每个描述列表可以由多个`<dd>`和`<dt>`组成

###### Description Term

`<dt>` 用于标识一个*描述术语*，作为描述的小标题

###### Description Definition

`<dd>` 用于标识具体的描述部分

##### 引用

###### 块引用

`<blockquote>` 

###### 行内引用

`<q>`

###### 引文

`<cite>` 标签用于标注引文，可以与`<a>`配合使用

其他的标签也有cite属性，该属性内容只有配合JS或CSS才会显示

##### 缩略语

`<abbr>` 用于包裹一个缩写，如

```html
<p>我们使用 <abbr title="超文本标记语言（Hyper text Markup Language）">HTML</abbr> 来组织网页文档。</p>
```

显示时只会显示缩写，而光标放上去时才会显示title的内容

##### 联系方式

`<address>`

##### 上下标

`<sup>` 上标  `<sub>` 下标

##### 代码

包括 `<code> <pre> <var> <kbd> <samp>` 等标签

##### 时间

`<time>`

#### 多媒体

##### 图片

`<img>`

常用属性

* src  图片源

* alt  备选文本，在图片无法显示时会被显示，也可以被用于无障碍设计

* width height  宽高

* title  标题

`<figure>`

h5元素，用于为图片提供一个块级的语义容器（只是作为容器，因此内容可以包含图片以及其他元素）

``<figureaction>``

是对于`<figure>`内容的说明

`<picture>`

一般用于响应式图片，即通过设备屏幕的分辨率来选择加载的图片源

##### 视频

`<video>`

常用属性

* src  视频源

* controls  若指定了该属性，则使用浏览器的默认控件控制视频播放等

由于音视频涉及编码和格式问题（复杂性可以见[原文](https://developer.mozilla.org/zh-CN/docs/Learn/HTML/Multimedia_and_embedding/Video_and_audio_content)），所以可以使用`<source>`标签为其提供不同的源

##### 音频

`<audio>` 与视频使用方式大体相同

##### 字幕

`<track>` 标签用于指定字母使用的WebVTT文件

#### 嵌入元素

`<iframe>` 可以通过一些共享的设置来在自己网站上嵌入其他网站的内容。但由于是在自己的网页上运行别人的网页，因此可能带来安全问题，浏览器也针对这些问题进行了一些限制（如沙盒）

## 一些语法

### 特殊字符转义

html使用字符转义如下

| 原字符 | 转义字符     |
| --- | -------- |
| <   | `&lt;`   |
| >   | `&gt;`   |
| "   | `&quot;` |
| '   | `&apos;` |
| &   | `&amp;`  |

### 注释

`<!-- xxx -->`
