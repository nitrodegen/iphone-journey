#include <stdio.h>
//#include <elf.h> // stupid macOS doesnt have elf.h for some reason??
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <sys/mman.h>
#include <algorithm>

using namespace std;
#define TYPE_OF_ELF 4// EI_CLASS
typedef struct {
        unsigned char   e_ident[16];
        uint16_t      e_type;
        uint16_t      e_machine;
        uint32_t      e_version;
        uint32_t      e_entry;
        uint32_t       e_phoff;
        uint32_t       e_shoff;
        uint32_t      e_flags;
        uint16_t      e_ehsize;
        uint16_t      e_phentsize;
        uint16_t      e_phnum;
        uint16_t      e_shentsize;
        uint16_t      e_shnum;
        uint16_t      e_shstrndx;
} elf32_header;

typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Word;
typedef uint32_t Elf32_Addr;
typedef uint32_t Elf32_Off;

typedef struct
{
  Elf32_Word	sh_name;		/* Section name (string tbl index) */
  Elf32_Word	sh_type;		/* Section type */
  Elf32_Word	sh_flags;		/* Section flags */
  Elf32_Addr	sh_addr;		/* Section virtual addr at execution */
  Elf32_Off	sh_offset;		/* Section file offset */
  Elf32_Word	sh_size;		/* Section size in bytes */
  Elf32_Word	sh_link;		/* Link to another section */
  Elf32_Word	sh_info;		/* Additional section information */
  Elf32_Word	sh_addralign;		/* Section alignment */
  Elf32_Word	sh_entsize;		/* Entry size if section holds table */
} section_header;

typedef struct
{
  unsigned char	e_ident[16];	/* Magic number and other info */
  Elf32_Half	e_type;			/* Object file type */
  Elf32_Half	e_machine;		/* Architecture */
  Elf32_Word	e_version;		/* Object file version */
  Elf32_Addr	e_entry;		/* Entry point virtual address */
  Elf32_Off	e_phoff;		/* Program header table file offset */
  Elf32_Off	e_shoff;		/* Section header table file offset */
  Elf32_Word	e_flags;		/* Processor-specific flags */
  Elf32_Half	e_ehsize;		/* ELF header size in bytes */
  Elf32_Half	e_phentsize;		/* Program header table entry size */
  Elf32_Half	e_phnum;		/* Program header table entry count */
  Elf32_Half	e_shentsize;		/* Section header table entry size */
  Elf32_Half	e_shnum;		/* Section header table entry count */
  Elf32_Half	e_shstrndx;		/* Section header string table index */
} eheader;
#define MMAPPED_ADDR 0x3020

int main(int argc, char *argv[] ){
      if(argc < 2 ) {
        printf("\n provide filename to create shellcode.");
        exit(1);
      }
      printf("\n*** payload extractor , tool for jailbreaking iPhone 3G. ***\n*** NOTE: payload should not contain any words. cause they may not be saved. ***");
      size_t size_of_file = 0;
      struct stat st;
      stat(argv[1],&st);
      int fd = open(argv[1],O_RDONLY);
      char *mp = (char*)mmap((void*)MMAPPED_ADDR,st.st_size,PROT_READ,MAP_PRIVATE,fd,0);
      eheader * ehead = (eheader*)mp;
      section_header *header = (section_header*)(mp+ehead->e_shoff);
      section_header *shdr = &header[ehead->e_shstrndx];
      const char *const constrp =mp+shdr->sh_offset;

      int payload_size = 0; 
      int addr =0 ;

      for(int i =0;i<ehead->e_shnum;i++){
        string n = constrp+header[i].sh_name;
        if(n == ".text"){
          payload_size = header[i].sh_size;
          addr = header[i].sh_addr;

          break;
        }
      } 
	int k =0;
	unsigned char* pp =(unsigned char*)malloc(payload_size);
	for(int i = 52+addr;i<payload_size+52+addr;i++){
		pp[k] = mp[i];
		printf("\n%x",pp[k]);	
		k++;
	}
	FILE *a = fopen("payload.bin","wb");
	fwrite(pp,sizeof(unsigned char),payload_size,a);
	fclose(a);

}	   



