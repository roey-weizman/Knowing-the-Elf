#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <elf.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

int Currentfd;
void *map_start;
char* fileName=NULL;
char str[100];
struct stat myStat;
Elf32_Ehdr *myHeader;
Elf32_Shdr *mySecHeader;
Elf32_Sym * symbolTable;
int debug=0;




char* typeToString(int type){
	switch(type){
		case 0: 
				return "NULL";
		case 1:
				return "PROGBITS";
		case 2:
				return "SYMTAB";
		case 3:
				return "STRTAB";
		case 4:
				return "RELA";
		case 5:
				return "HASH";
		case 6:
				return "DYNAMIC";
		case 7:
				return "NOTE";
		case 8:
				return "NOBITS";
		case 9:
				return "REL";
		case 10:
				return "SHLIB";
		case 11: 
				return "DYNSIM";
		default: 
		return "";
		}
}	


void printSYMTAB(){
	Elf32_Shdr *string_table;
	int sectionNum = myHeader->e_shnum;//num of entries(sections) 	
	int i=0;
	//get symbol table
	for (;i < sectionNum; i++){
	    if (mySecHeader[i].sh_type == SHT_SYMTAB) {
	        symbolTable = (Elf32_Sym *)((char *)map_start + mySecHeader[i].sh_offset);
	        break;}
	    }

	  //get related string table
    for(int k=0;k<sectionNum;k++){
    	if (mySecHeader[k].sh_type == SHT_STRTAB) {
    				string_table=&mySecHeader[k];
    	}
    }

  	const char *const stringTableOffset = map_start + string_table->sh_offset;

    int symbolTableEntries= mySecHeader[i].sh_size/sizeof(Elf32_Sym);

    printf("Symbol table '.symtab' contains %d entries:\n",symbolTableEntries);
    printf("index   value  section_index section_name  symbol_name\n");

    for(int j=0;j<symbolTableEntries;j++){
    	printf("[%2d] %8x	 %d 		 SYMTAB   %s\n",j,symbolTable[j].st_value,symbolTable[j].st_shndx,stringTableOffset+symbolTable[j].st_name );
    }
    printf("\n\n");
}

void printDYNSYM(){
	Elf32_Shdr *string_table;
	int sectionNum = myHeader->e_shnum;//num of entries(sections) 	
	int i=0;
	//get symbol table
	for (;i < sectionNum; i++){
	    if (mySecHeader[i].sh_type == SHT_DYNSYM) {
	        symbolTable = (Elf32_Sym *)((char *)map_start + mySecHeader[i].sh_offset);
	        break;
	    }
	    }

	  //get related string table
    for(int k=0;k<sectionNum;k++){
    	if (mySecHeader[k].sh_type == SHT_STRTAB) {
    				string_table=&mySecHeader[k];
    				break;
    	}

    }

  	const char *const stringTableOffset = map_start + string_table->sh_offset;

    int symbolTableEntries= mySecHeader[i].sh_size/sizeof(Elf32_Sym);

    printf("Symbol table '.dynsym' contains %d entries:\n",symbolTableEntries);
    printf("index   value  section_index section_name  symbol_name\n");

    for(int j=0;j<symbolTableEntries;j++){
    	printf("[%2d] %8x	 %d 		DYNSYM   %s\n",j,symbolTable[j].st_value,symbolTable[j].st_shndx,stringTableOffset+symbolTable[j].st_name );
    }
    printf("\n\n");
}

void PrintSymbols(){
	printDYNSYM();
	printSYMTAB();

}

void quit(){
	if(!map_start)
		munmap(map_start, myStat.st_size);
	if(!Currentfd)
	close(Currentfd);
	printf("Quitting...\n");
	exit(0);
}

void ExamineELFFile(){
	printf("Enter a file name:\n");
	gets(str);
	fileName=strdup(str);
	if(Currentfd!=NULL)
		close(Currentfd);
	Currentfd=open(fileName, O_RDWR);
	 if( Currentfd < 0 ) {
      perror("error in open");
      exit(-1);
   }
    if( fstat(Currentfd, &myStat) != 0 ) {
      perror("stat failed");
      exit(-1);
   }
   if ( (map_start = mmap(0, myStat.st_size, PROT_READ, MAP_PRIVATE, Currentfd, 0)) == MAP_FAILED ) {
      perror("mmap failed");
      exit(-4);
   }
   myHeader = (Elf32_Ehdr *) map_start;
   	mySecHeader= (Elf32_Shdr *)(map_start + myHeader->e_shoff);//eshof-offset for section table

   printFileDetails();
}

printFileDetails(){
  printf("\nFile Details:\n");
  printf("Magic number: %d %d %d \n", myHeader->e_ident[0],myHeader->e_ident[1],myHeader->e_ident[2]);
  if(myHeader->e_ident[5]==1)
  	printf("Data Encoding: Little Endian\n");
  else printf("Data Encoding: Big Endian\n");
  printf("Entry Point: %0x \n",myHeader->e_entry);
  printf("Start of section header: %d \n",myHeader->e_shoff);
  printf("Number of section header: %d \n", myHeader->e_shnum);
  printf("Size of section header: %d\n",myHeader->e_shentsize);
  printf("Start of program header: %d\n",myHeader->e_phoff);
  printf("Number of program header: %d\n",myHeader->e_phnum);
  printf("Size of program header: %d\n",myHeader->e_phentsize);

  


}
void PrintSectionNames(){
	if(!Currentfd){
		perror("FIle wasn't Open\n");
	      exit(-1);
	}
  	int sectionNum = myHeader->e_shnum;//num of entries(sections)

  Elf32_Shdr *string_table = &mySecHeader[myHeader->e_shstrndx];
  const char *const string_table_of_map = map_start + string_table->sh_offset;//sh_ofsset- begining of file to the first byte in section
  for (int i = 0; i < sectionNum; ++i) {
  	char* type =typeToString(mySecHeader[i].sh_type);
    printf("[%2d]: %s %08x %08x %08x %s \n", i, string_table_of_map + mySecHeader[i].sh_name,mySecHeader[i].sh_addr,mySecHeader[i].sh_offset,mySecHeader[i].sh_size,type);
  }

} 	


void ToggleDebugMode() {
  if(!debug){
    debug=1;
    printf("Debug flag now on\n");
	}
  else{
  	debug=0;
  	printf("Debug flag now off\n");
  }
    
}
typedef struct fun_desc {
 	char *name;
  	void (*fun)();
}fun_desc ;
int main(){
	
	 int bounds=5;
	struct fun_desc menu[] = {{ "Toggle Debug Mode", &ToggleDebugMode }, { "Examine ELF File", &ExamineELFFile } ,{"Print Section Names",&PrintSectionNames},{"Print Symbols",&PrintSymbols},{ "Quit", &quit },{ NULL, NULL } };	
	while(1){
		if(debug){
			
		}
		printf("Choose action:\n");
		for(int i=0;i<bounds;i++){
			printf("%d) %s\n",i,menu[i].name);
		}
		int n; 
		printf("Option: ");
		n=(getc(stdin)-'0');
		if(n>=0&&n<=bounds)
			printf("Within bounds\n\n");
		else {
			printf("Not within bounds\n");
			return -1;
		}
		getc(stdin);
		(menu[n].fun)();
		

		printf("\n\n");
	}
	return 0;
}
