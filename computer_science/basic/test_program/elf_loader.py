import struct
import sys
import getopt

class ELF_header(object):
    ELFCLASS = {0:"ELFCLASSNONE",1:"ELFCLASS32",2:"ELFCLASS64"}
    ELFDATA = {0:"ELFDATANONE",1:"ELFDATA2LSB",2:"ELFDATA2MSB"}
    E_TYPE = {0:"ET_NONE",1:"ET_REL",2:"ET_EXEC",3:"ET_DYN",4:"ET_CORE",0xff00:"ET_LOPROC",0xffff:"ET_HIPROC"}
    E_MACHINE = {0:"EM_NONE",1:"EM_M32",2:"EM_SPARC",3:"EM_386",4:"EM_68K",5:"EM_88K",7:"EM_860",8:"EM_MIPS",20:"EM_PowerPC",22:"EM_S390",40:"EM_ARM",42:"EM_SuperH",50:"EM_IA64",62:"EM_X86_64",183:"EM_AArch64",243:"EM_RISC_V"}

    def __init__(self,in_str):
        tmp = struct.unpack("<16c",in_str[:16])
        self.e_ident = tmp
        self.handle_data()
        if self.width == 32:
            tmp = struct.unpack("<HHIIIIIHHHHHH",in_str[16:52])
        else:
            tmp = struct.unpack("<HHIQQQIHHHHHH",in_str[16:])
        self.e_type = self.E_TYPE[tmp[0]]
        self.e_machine = self.E_MACHINE.setdefault(tmp[1],"ET_NONE")
        self.e_version = tmp[2]
        self.e_entry = tmp[3]
        self.e_phoff = tmp[4]
        self.e_shoff = tmp[5]
        self.e_flags = tmp[6]
        self.e_ehsize = tmp[7]
        self.e_phentsize = tmp[8]
        self.e_phnum = tmp[9]
        self.e_shentsize = tmp[10]
        self.e_shnum = tmp[11]
        self.e_shstrndx = tmp[12]
        if self.ei_version != 1 or self.ei_version != self.e_version:
            print "Invaild ELF Version"
            raise TypeError

    def __repr__(self):
        ret = ""
        ret += "EI_CLASS:%s\tEI_DATA:%s\tE_TYPE:%s\tE_MACHINE:%s\n" %(self.ei_class,self.ei_data,self.e_type,self.e_machine)
        ret += "E_ENTRY:%s\nE_PHOFF:%s\nE_PHNUM:%s\nE_PHENTSIZE:%s\n\n" %(hex(self.e_entry),self.e_phoff,self.e_phnum,self.e_phentsize)
        ret += "E_SHOFF:%s\nE_SHNUM:%s\nE_SHENTSIZE:%s\n" %(self.e_shoff,self.e_shnum,self.e_shentsize)
        return ret

    __str__ = __repr__

    def handle_data(self):
        magic = "".join(self.e_ident[0:4])
        if magic != "\x7fELF":
            print "Not A ELF File:Magic Number Wrong"
            raise TypeError

        self.ei_class = self.ELFCLASS[ord(self.e_ident[4])]
        if self.ei_class == "ELFCLASS64":
            self.width = 64
        elif self.ei_class == "ELFCLASS32":
            self.width = 32
        else:
            self.width = 0
            print "Only 32/64bit ELF Can be Read"
            raise TypeError
        
        self.ei_data = self.ELFDATA[ord(self.e_ident[5])]
        self.ei_version = ord(self.e_ident[6])
        if self.ei_data != "ELFDATA2LSB":
            print "Only LSB ELF Can be Read"
            raise TypeError

class Program_header(object):
    P_TYPE = {0:"PT_NULL",1:"PT_LOAD",2:"PT_DYNAMIC",3:"PT_INTERP",4:"PT_NOTE",5:"PT_SHLIB",6:"PT_PHDR"}
    P_FLAG = {1:"PF_X",2:"PF_W",4:"PF_R"}
    def __init__(self,in_str):
        if len(in_str) == 32:
            self.width = 32
            tmp = struct.unpack("<IIIIIIII",in_str)
            self.p_type = self.P_TYPE.setdefault(tmp[0],str(tmp[0]))
            self.p_offset = tmp[1]
            self.p_vaddr = tmp[2]
            self.p_paddr = tmp[3]
            self.p_filesz = tmp[4]
            self.p_memsz = tmp[5]
            flag_tmp = tmp[6]&0x7
        else:
            self.width = 64
            tmp = struct.unpack("<IIQQQQQQ",in_str)
            self.p_type = self.P_TYPE.setdefault(tmp[0],str(tmp[0]))
            flag_tmp = tmp[1]&0x7
            self.p_offset = tmp[2]
            self.p_vaddr = tmp[3]
            self.p_paddr = tmp[4]
            self.p_filesz = tmp[5]
            self.p_memsz = tmp[6]

        mask = 0x4
        self.p_flags = ""
        for i in xrange(3):
            if flag_tmp & mask != 0:
                self.p_flags += self.P_FLAG[flag_tmp&mask] + " "
                mask >>= 1
        self.p_align = tmp[7]

    def __repr__(self):
        ret = ""
        ret += "P_TYPE:%s\tP_OFFSET:%s\nP_VADDR:%s\tP_PADDR:%s\nP_FILESZ:%s\tP_MEMSZ:%s\n" %(self.p_type,hex(self.p_offset),hex(self.p_vaddr),hex(self.p_paddr),hex(self.p_filesz),hex(self.p_memsz))
        return ret
    __str__ = __repr__

class Section_header(object):
    SH_TYPE = {0:"SHT_NULL",1:"SHT_PROGBITS",2:"SHT_SYMTAB",3:"SHT_STRTAB",4:"SHT_RELA",5:"SHT_HASH",6:"SHT_DYNAMIC",7:"SHT_NOTE",8:"SHT_NOBITS",9:"SHT_REL",10:"SHT_SHLIB",11:"SHT_DYNSYM",0x70000000:"SHT_LOPROC",0x7fffffff:"SHT_HIPROC",0x80000000:"SHT_LOUSER",0x8fffffff:"SHT_HIUSER"}
    SH_FLAGS = {1:"SHF_WRITE",2:"SHF_ALLOC",4:"SHF_EXECINSTR"}
    def __init__(self,in_str):
        if len(in_str) == 40:
            self.width = 32
            tmp = struct.unpack("<IIIIIIIIII",in_str)
        else:
            self.width = 64
            tmp = struct.unpack("<IIQQQQIIQQ",in_str)
        self.sh_name = tmp[0]
        self.sh_type = self.SH_TYPE.setdefault(tmp[1],tmp[1])
        sh_flags_tmp = tmp[2] & 0x7
        mask = 0x04
        self.sh_flags = ""
        for i in xrange(3):
            if sh_flags_tmp & mask:
                self.sh_flags += self.SH_FLAGS[sh_flags_tmp&mask] + " "
                mask >>= 1
        self.sh_addr = tmp[3]
        self.sh_offset = tmp[4]
        self.sh_size = tmp[5]
        self.sh_link = tmp[6]
        self.sh_info = tmp[7]
        self.sh_addralign = tmp[8]
        self.sh_entsize = tmp[9]
        self.section_name = ""                          # used to place section name which read from .shstrndx

    def read_section(self,f):
        offset = i.sh_offset
        f.seek(offset)
        data_raw = f.read(i.sh_size)
        return data_raw

    def __repr__(self):
        ret = ""
        if self.section_name != "":
            ret += "Name:%s\t" %self.section_name
        ret += "Type:%s\tAddr:%s\tOffset:%s\tSize:%s\tst_info:%s\tst_link:%s" %(self.sh_type,hex(self.sh_addr),hex(self.sh_offset),hex(self.sh_size),self.sh_info,self.sh_link)
        return ret    
    __str__ = __repr__

class symtab_symbol(object):
    ST_INFO = {0:"STT_NOTYPE",1:"STT_OBJECT",2:"STT_FUNC",3:"STT_SECTION",4:"STT_FILE"}
    STB = {0:"STB_LOCAL",1:"STB_GLOBAL",2:"STB_WEAK"}
    SHN = {0xfff1:"SHN_ABS",0xfff2:"SHN_COMMON",0:"SHN_UNDEF"}
    def __init__(self,in_str):
        if len(in_str) == 16:
            tmp = struct.unpack("<IIIBBH",in_str)
        else:
            tmp = struct.unpack("<IQIBBH",in_str)
        self.st_name = tmp[0]
        self.st_value = tmp[1]
        self.st_size = tmp[2]
        raw_info = tmp[3]
        symbol_type = self.ST_INFO.setdefault(raw_info&0xf,raw_info&0xf)
        symbol_bind = self.ST_INFO.setdefault( (raw_info>>4)&0xf, (raw_info>>4)&0xf )
        self.st_info = (symbol_bind,symbol_type)
        self.st_other = tmp[4]
        self.st_shndx = tmp[5]      # the index of section where symbols defined 

class ELF_file(ELF_header):
    def __init__(self,file_obj):
            elfheader_raw = f.read(64)
            self.elf_header = ELF_header.__init__(self,elfheader_raw)
            self.phtable = []
            self.shtable = []
            self.symtable = None
            f.seek(self.e_phoff)
            phtable_raw = f.read(self.e_phentsize * self.e_phnum)
            for i in xrange(self.e_phnum):              # construct program table
                l = i*self.e_phentsize
                r = (i+1)*self.e_phentsize
                tmp = Program_header(phtable_raw[l:r])
                self.phtable.append(tmp)

            f.seek(self.e_shoff)
            shtable_raw = f.read(self.e_shentsize * self.e_shnum)
            for i in xrange(self.e_shnum):              # construct section table
                l = i*self.e_shentsize
                r = (i+1)*self.e_shentsize
                tmp = Section_header(shtable_raw[l:r])
                self.shtable.append(tmp)
            section_shstrndx = self.shtable[self.e_shstrndx]
            f.seek(section_shstrndx.sh_offset)
            shstrndx_raw = f.read(section_shstrndx.sh_size)
            for i in xrange(self.e_shnum):              # read section name from shstrndx section
                l = self.shtable[i].sh_name
                r = l
                while shstrndx_raw[r] != '\x00':
                    r += 1
                self.shtable[i].section_name = shstrndx_raw[l:r]

    def get_sym(self,num,f):
        if num < len(self.shtable):
            i = self.shtable[num]
            sym_raw = i.read_section(f)
            if self.width == 32:
                pass
        return None


    def read_reloc(self,f):
        R_RLOC_32 = {0:"R_386_NONE",1:"R_386_32",2:"R_386_PC32",3:"R_386_GOT32",4:"R_386_PLT32",5:"R_386_COPY32",6:"R_386_GLOB_DAT",7:"R_386_JMP_SLOT",8:"R_386_RELATIVE",9:"R_386_GOTOFF",10:"R_386_GOTPC",11:"R_386_32PLT",42:"R_386_IRELATIVE"}
        R_RLOC_64 = {0:"R_AMD64_NONE",1:"R_AMD64_64",2:"R_AMD64_PC32",3:"R_AMD64_GOT32",4:"R_AMD64_PLT32",5:"R_AMD64_COPY",6:"R_AMD64_GLOB_DAT",7:"R_AMD64_JUMP_SLOT",8:"R_AMD64_RELATIVE",9:"R_AMD64_GOTPCREL",10:"R_AMD64_32",11:"R_AMD64_32s"}
        self.reloc = []
        self.reloca = []
        for i in self.shtable:
            if i.sh_type == "SHT_RELA" or i.sh_type == "SHT_REL":
                reloc_raw = i.read_section(f)
                if i.sh_type == "SHT_RELA":
                    i.symtab = self.get_sym(i.sh_link,f)
                    if self.width == 32:
                        step = 12
                        pattern = "<IIi"
                    else:
                        step = 24
                        pattern = "<QQq"
                    length = i.sh_size / step
                    for i in xrange(length):
                        r = i * step
                        l = (i+1) * step
                        tmp = struct.unpack(pattern,reloc_raw[r:l])
                        r_offset = tmp[0]
                        r_info = tmp[1]
                        r_append = tmp[2]
                        if self.width == 32:
                            r_sym = r_info >> 8
                            r_type = R_RLOC_32[ r_info & 0xff ]
                        else:
                            r_sym = r_info >> 32
                            r_type = R_RLOC_64[ r_info & 0xffffffff ]

                        self.reloca.append( (tmp[0],tmp[1],tmp[2],r_type) )
                elif i.sh_type == "SHT_REL":
                    i.symtab = self.get_sym(i.sh_link,f)
                    if self.width == 32:
                        step = 8
                        pattern = "<II"
                    else:
                        step = 16
                        pattern = "<QQ"
                    length = i.sh_size / step
                    for i in xrange(length):
                        r = i * step
                        l = (i+1) * step
                        tmp = struct.unpack(pattern,reloc_raw[r:l])
                        r_offset = tmp[0]
                        r_info = tmp[1]
                        if self.width == 32:
                            r_sym = r_info >> 8
                            r_type = R_RLOC_32[ r_info & 0xff ]
                        else:
                            r_sym = r_info >> 32
                            r_type = R_RLOC_64[ r_info & 0xffffffff ]
                        self.reloc.append( (tmp[0],tmp[1],r_type) )
                        
    
    def print_reloc(self):
        if self.reloc != []:
    
            for (offset,info,r_type) in self.reloc:
                print "%s\t\t%s\t%s" %(hex(offset),hex(info),r_type)
        if self.reloca != []:
            for (offset,info,append,r_type) in self.reloca:
                print "%s\t\t%s\t%s\t%s" %(hex(offset),hex(info),append,r_type)

    def section2segment(self):
        start = 0
        print "====Section to Segment mapping:===="
        for i in self.phtable:
            program_section = []
            l = i.p_offset
            r = i.p_filesz + l
            for tmp in self.shtable:
                if tmp.sh_offset >= l and (tmp.sh_offset+tmp.sh_size) <= r:
                    program_section.append(tmp.section_name)
            for i in program_section:
                print i,
            print ""


def Usage():
    print "python elf_loader.py [opt] -i file"
    print "-a  all"
    print "-h  file header"
    print "-S  section header"
    print "-l  program header"
    print "-r  relocations"
    print "-H  Help"
    exit(0)

if __name__ == "__main__":
    try:
        opts,args = getopt.getopt(sys.argv[1:],"ahSlrHi:")
    except getopt.GetoptError:
        Usage()

    file_header = True
    section_header = False
    program_header = False
    relocation = False
    for (opt,arg) in opts:
        if opt in '-a':
            section_header = True
            program_header = True
            relocation = True
        elif opt in "-h":
            file_header = True
        elif opt in "-S":
            section_header = True
        elif opt in "-l":
            program_header = True
        elif opt in "-r":
            relocation = True
        elif opt in "-H":
            Usage()
        elif opt in "-i":
            myfile = arg

    if myfile == "":
        Usage()
    
    try:
        f = open(myfile,"rb")
        elf_file = ELF_file(f)

        if file_header:
            print "====ELF HEADER===="
            print elf_file
        if section_header:
            print "====SECTION HEADER===="
            n = 0
            for i in elf_file.shtable:
                print str(n)+"\t",i
                n+=1
        if program_header:
            print "====PROGRAM HEADER===="
            for i in elf_file.phtable:
                print i
            elf_file.section2segment()
        if relocation:
            print "====Relocation Section===="
            elf_file.read_reloc(f)
            elf_file.print_reloc()
    finally:
        f.close()
