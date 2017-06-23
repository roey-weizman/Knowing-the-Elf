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
int debug=0;

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
	mySecHeader= (Elf32_Shdr *)(map_start + myHeader->e_shoff);//eshof-offset for section table
  	int sectionNum = myHeader->e_shnum;//num of entries(sections)

  Elf32_Shdr *string_table = &mySecHeader[myHeader->e_shstrndx];
  const char *const string_table_of_map = map_start + string_table->sh_offset;//sh_ofsset- begining of file to the first byte in section
  for (int i = 0; i < sectionNum; ++i) {
    printf("[%2d]: %s %08x %08x %08x %s \n", i, string_table_of_map + mySecHeader[i].sh_name,mySecHeader[i].sh_addr,mySecHeader[i].sh_size,mySecHeader[i].sh_type);
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
	
	 int bounds=4;
	struct fun_desc menu[] = {{ "Toggle Debug Mode", &ToggleDebugMode }, { "Examine ELF File", &ExamineELFFile } ,{"Print Section Names",&PrintSectionNames},{ "Quit", &quit },{ NULL, NULL } };	
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
