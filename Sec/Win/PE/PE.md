#PE文件格式

##概览


![im01](im01.jpg)


##1.MS-DOS头
**头文件**
	WINNT.H

	typedef struct _IMAGE_DOS_HEADER	  // DOS下的.EXE文件头
     {
		USHORT e_magic;         // 魔数						+00
 	    USHORT e_cblp;          // 文件最后一页的字节数				+02
 	    USHORT e_cp;            // 文件的页数					+04
 	    USHORT e_crlc;          // 重定位						+06
 	    USHORT e_cparhdr;       // 段中头的大小					+08
 	    USHORT e_minalloc;      // 需要的最少额外段					+0A
 	    USHORT e_maxalloc;      // 需要的最多额外段					+0C
	    USHORT e_ss;            // 初始的(相对的)SS寄存器值				+0E
	     USHORT e_sp;            // 初始的SP寄存器值				+10
	     USHORT e_csum;          // 校验和						+12
	     USHORT e_ip;            // 初始的IP寄存器值				+14
	     USHORT e_cs;            // 初始的(相对的)CS寄存器值			+16
	     USHORT e_lfarlc;        // 重定位表在文件中的地址				+18
	     USHORT e_ovno;          // 交叠数						+1A
	     USHORT e_res[4];        // 保留字						+1C
	     USHORT e_oemid;         // OEM识别符(用于e_oeminfo成员)			+24
	     USHORT e_oeminfo;       // OEM信息; e_oemid中指定的			+26
	     USHORT e_res2[10];      // 保留字						+28
	     LONG   e_lfanew;        // 新exe头在文件中的地址				+3C
	} IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;


###重点字段
	e_magic		#define IMAGE_DOS_SIGNATURE 0x5A4D	//'M' 'Z'	+00
	e_lfanew	//PE头偏移量	+3C
##2.PE文件头
**头文件**
WINNT.H

###文件头
	typedef struct _IMAGE_NT_HEADERS {
		DWORD Signature;				//+00
		IMAGE_FILE_HEADER FileHeader;			//+04
		IMAGE_OPTIONAL_HEADER32 OptionalHeader;		//+18
	}IMAGE_NT_HEADERS;

####Signature
	#define IMAGE_DOS_SIGNATURE             0x5A4D      // MZ	DOS
	#define IMAGE_OS2_SIGNATURE             0x454E      // NE	OS/2
	#define IMAGE_OS2_SIGNATURE_LE          0x454C      // LE	OS/2 LE
	#define IMAGE_NT_SIGNATURE              0x00004550  // PE00	WINNT
	
####IMAGE_FILE_HEADER

	typedef struct _IMAGE_FILE_HEADER {
	     USHORT  Machine;                 //机器			+00
	     USHORT  NumberOfSections;        //节数			+02
	     ULONG   TimeDateStamp;           //时间日期戳		+04
	     ULONG   PointerToSymbolTable;    //符号表指针		+08
	     ULONG   NumberOfSymbols;         //符号数			+0C
	     USHORT  SizeOfOptionalHeader;    //可选头的大小		+10
	     USHORT  Characteristics;         //特性			+12
	} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

####IMAGE_OPTIONAL_HEADER

###PE头位置
	PNTHeader = ImageBase + dosHeader->e_lfanew

##3.PE可选头
	typedef struct _IMAGE_OPTIONAL_HEADER {
     		//
     		//标准域 
     		//
     		USHORT  Magic;                   //魔数			+00
     		UCHAR   MajorLinkerVersion;      //链接器主版本号	+02
     		UCHAR   MinorLinkerVersion;      //链接器小版本号	+03
     		ULONG   SizeOfCode;              //代码大小		+04
     		ULONG   SizeOfInitializedData;   //已初始化数据大小	+08
     		ULONG   SizeOfUninitializedData; //未初始化数据大小 	+0C
     		ULONG   AddressOfEntryPoint;     //入口点地址		+10
     		ULONG   BaseOfCode;              //代码基址		+14
     		ULONG   BaseOfData;              //数据基址		+18
     		//
     		//NT增加的域
     		//
     		ULONG   ImageBase;                  //映像文件基址	+1C
     		ULONG   SectionAlignment;           //节对齐		+20
     		ULONG   FileAlignment;              //文件对齐		+24
     		USHORT  MajorOperatingSystemVersion;//操作系统主版本号	+28
     		USHORT  MinorOperatingSystemVersion;//操作系统小版本号	+3A
     		USHORT  MajorImageVersion;          //映像文件主版本号	+3C
     		USHORT  MinorImageVersion;          //映像文件小版本号	+3E
     		USHORT  MajorSubsystemVersion;      //子系统主版本号	+40
     		USHORT  MinorSubsystemVersion;      //子系统小版本号	+42
     		ULONG   Reserved1;                  //保留项1		+44
     		ULONG   SizeOfImage;                //映像文件大小	+48
     		ULONG   SizeOfHeaders;              //所有头的大小	+4C
     		ULONG   CheckSum;                   //校验和		+50
     		USHORT  Subsystem;                  //子系统		+54
     		USHORT  DllCharacteristics;         //DLL特性		+56
     		ULONG   SizeOfStackReserve;         //保留栈的大小	+58
     		ULONG   SizeOfStackCommit;          //指定栈的大小	+5C
     		ULONG   SizeOfHeapReserve;          //保留堆的大小	+60
     		ULONG   SizeOfHeapCommit;           //指定堆的大小	+64
     		ULONG   LoaderFlags;                //加载器标志	+68
     		ULONG   NumberOfRvaAndSizes;        //RVA的数量和大小	+6C
     		IMAGE_DATA_DIRECTORY DataDirectory  [IMAGE_NUMBEROF_DIRECTORY_ENTRIES];   //数据目录数组  +70
	} IMAGE_OPTIONAL_HEADER, *PIMAGE_OPTIONAL_HEADER;

	
##数据目录
	// 各个目录项
	// 输出目录
	#define IMAGE_DIRECTORY_ENTRY_EXPORT         0
	// 输入目录
	#define IMAGE_DIRECTORY_ENTRY_IMPORT         1
	// 资源目录
	#define IMAGE_DIRECTORY_ENTRY_RESOURCE       2
	// 异常目录
	#define IMAGE_DIRECTORY_ENTRY_EXCEPTION      3
	// 安全目录
	#define IMAGE_DIRECTORY_ENTRY_SECURITY       4
	// 基址重定位表
	#define IMAGE_DIRECTORY_ENTRY_BASERELOC      5
	// 调试目录
	#define IMAGE_DIRECTORY_ENTRY_DEBUG          6
	// 描述字符串
	#define IMAGE_DIRECTORY_ENTRY_COPYRIGHT      7
	// 机器值(MIPS GP)
	#define IMAGE_DIRECTORY_ENTRY_GLOBALPTR      8
	// TLS（线程本地存储）⑥目录
	#define IMAGE_DIRECTORY_ENTRY_TLS            9
	// 载入配置目录
	#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG    10

