core文件夹包含几个顶层的核心逻辑

* 参数解析

* 搜索

## app.rs

使用clap包定义了一个命令行处理程序

所有支持的选项及其help都在该文件下

## main.rs

## args.rs

### 枚举

#### Command

```
Search            单线程搜索
SearchParallel    多线程搜索
SearchNever       进行搜索，且可能无法搜索到匹配（如--max-count=0的情况）
Files             列出要搜索的文件，单线程
FilesParallel     列出要搜索的文件，多线程
Types             列出所有要搜索的文件类型
PCRE2Version      打印PCRE2引擎的版本
```
