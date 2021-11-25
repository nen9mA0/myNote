#include <stdio.h>
#include <assert.h>
#include <stdint.h>

#include <windows.h>


// === copy from /usr/include/elf.h ===

/* Type for a 16-bit quantity.  */
typedef uint16_t Elf64_Half;

/* Types for signed and unsigned 32-bit quantities.  */
typedef uint32_t Elf64_Word;

/* Type of addresses.  */
typedef uint64_t Elf64_Addr;

/* Type of file offsets.  */
typedef uint64_t Elf64_Off;


#define EI_NIDENT (16)

typedef struct
{
  unsigned char e_ident[EI_NIDENT];     /* Magic number and other info */
  Elf64_Half    e_type;                 /* Object file type */
  Elf64_Half    e_machine;              /* Architecture */
  Elf64_Word    e_version;              /* Object file version */
  Elf64_Addr    e_entry;                /* Entry point virtual address */
  Elf64_Off     e_phoff;                /* Program header table file offset */
  Elf64_Off     e_shoff;                /* Section header table file offset */
  Elf64_Word    e_flags;                /* Processor-specific flags */
  Elf64_Half    e_ehsize;               /* ELF header size in bytes */
  Elf64_Half    e_phentsize;            /* Program header table entry size */
  Elf64_Half    e_phnum;                /* Program header table entry count */
  Elf64_Half    e_shentsize;            /* Section header table entry size */
  Elf64_Half    e_shnum;                /* Section header table entry count */
  Elf64_Half    e_shstrndx;             /* Section header string table index */
} Elf64_Ehdr, *P_Elf64_Ehdr;


void help()
{
    printf("header.exe filename\n");
    exit(0);
}

void PrintPE(PIMAGE_NT_HEADERS64 p)
{
    if(p->Signature == IMAGE_NT_SIGNATURE)
    {
        printf("PE Header:\n");
        printf("Machine:      %x\n", p->FileHeader.Machine);
        printf("Section Num:  %x\n", p->FileHeader.NumberOfSections);
        printf("TimeStamp:    %x\n", p->FileHeader.TimeDateStamp);
        printf("Symbol Num:   %x\n", p->FileHeader.NumberOfSymbols);
        printf("Character:    %x\n", p->FileHeader.Characteristics);
    }
}

void PrintELF(P_Elf64_Ehdr p)
{
    printf("ELF Header:\n");
    printf("Machine:        %x\n", p->e_machine);
    printf("Type:           %x\n", p->e_type);
    printf("Version:        %x\n", p->e_version);
    printf("Flags:          %x\n", p->e_flags);
    printf("Program Header: %x\n", p->e_phnum);
    printf("Section Header: %x\n", p->e_shnum);
}

const char elf_magic[] = {0x7f, 'E', 'L', 'F'};

int main(int argc, char* argv[])
{
    PIMAGE_DOS_HEADER p_dos;
    PIMAGE_NT_HEADERS64 p_pe;
    P_Elf64_Ehdr p_elf;

    if(argc < 2)
        help();

    char *filename = argv[1];

    FILE *fp = fopen(filename, "rb");
    assert(fp);         // if file not exist, exit

    fseek(fp, 0, SEEK_END);
    size_t file_len = ftell(fp);  // get file length
    fseek(fp, 0, SEEK_SET);           // reset file pointer

    char *buf = malloc(file_len+10);    // allocate buffer

    fread(buf, file_len, 1, fp);        // read file into buffer

    p_dos = buf;
    p_elf = buf;
    if(p_dos->e_magic == 0x5a4d)        // whether is a PE file
    {
        p_pe = &buf[p_dos->e_lfanew];
        PrintPE(p_pe);
    }
    else if(!memcmp(p_elf->e_ident, elf_magic, 4))  // whether is a ELF file
    {
        PrintELF(p_elf);
    }
    else
    {
        printf("File Unreconized\n");
    }

    fclose(fp);
}
