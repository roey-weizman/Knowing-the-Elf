    #include <stdio.h>
    #include <string.h>
    #include <stdlib.h>
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
    Elf32_Shdr *string_table;
    Elf32_Rel * RelocationTable;

    int debug=0;

    typedef struct fun_desc {
        char *name;
        void (*fun)();
    }fun_desc ;

    int initializeRelTable(int type1,int type2,int index){
        int sectionNum = myHeader->e_shnum;//num of entries(sections)
        int i=index;
        //get symbol table
        for (;i < sectionNum; i++){
            if (mySecHeader[i].sh_type == type1||mySecHeader[i].sh_type == type2) {
                RelocationTable = (Elf32_Sym *)((char *)map_start + mySecHeader[i].sh_offset);
                break;
            }
        }
        return i;
    }
    int initializeCertainTable(int type){
        int sectionNum = myHeader->e_shnum;//num of entries(sections)
        int i=0;
        //get symbol table
        for (;i < sectionNum; i++){
            if (mySecHeader[i].sh_type == type) {
                symbolTable = (Elf32_Sym *)((char *)map_start + mySecHeader[i].sh_offset);
                break;
            }
        }
        return i;
    }

    void initializeStringTable(int type){
        int sectionNum = myHeader->e_shnum;//num of entries(sections)
        int i=0;
        //get symbol table
        for (;i < sectionNum; i++){
            if (mySecHeader[i].sh_type == type) {
                string_table=&mySecHeader[mySecHeader[i].sh_link];

            }
        }

    }
    void initializeStringTable1(int type1,int type2){
        int sectionNum = myHeader->e_shnum;//num of entries(sections)
        int i=0;
        //get symbol table
        for (;i < sectionNum; i++){
            if (mySecHeader[i].sh_type == type1||mySecHeader[i].sh_type == type2) {
                string_table=&mySecHeader[mySecHeader[i].sh_link];

            }
        }

    }
    char* intToType (int type){
        switch(type){
            case 0:
                return "R_386_NONE";
            case 1:
                return "R_386_32";
            case 2:
                return "R_386_PC32";
        }
    }

    int offsetToString(int offset){
        int name1=0; int name2=0;
        int index=initializeCertainTable(SHT_SYMTAB);
        initializeStringTable(SHT_SYMTAB);
        const char *const stringTableOffset = (char*)map_start + string_table->sh_offset+symbolTable[offset].st_name;
        if(strlen(stringTableOffset)>1)
            printf("   ");
        name1=printf("%s ",stringTableOffset);
        if(name1>1)
            return 1;

        index=initializeCertainTable(SHT_DYNSYM);
        initializeStringTable(SHT_DYNSYM);
        const char *const stringTableOffset1 = (char*)map_start + string_table->sh_offset+symbolTable[offset].st_name;
        if(strlen(stringTableOffset1)>1)
            printf("   ");
        name2=printf("%s ",stringTableOffset1);

        if(name1<=1&&name2<=1)
            return 0;
        return 1;

    }

    char* getSection(int offset){
        Elf32_Shdr* sections=(Elf32_Shdr *)((char*)map_start+myHeader->e_shoff);
        Elf32_Shdr* string_table1=&sections[myHeader->e_shstrndx];
        const char *const stringTableOffset = map_start + string_table1->sh_offset;


       return ((char*)(stringTableOffset))+sections[offset].sh_name;


    }
    void RelocationTablesRaw(){
        int sectionNum = myHeader->e_shnum; int index=-1; char* s;
        while(index<sectionNum) {
            index = initializeRelTable(SHT_REL, SHT_RELA, index+1);
            initializeStringTable1(SHT_DYNSYM,SHT_RELA);
            const char *const string_table_of_map1 = map_start + string_table->sh_offset;
            char* RelTabName=string_table_of_map1+mySecHeader[index].sh_name;
            int relAdd=mySecHeader[index].sh_offset;

            int relTableEntries = mySecHeader[index].sh_size / sizeof(Elf32_Rel);


            if(index<sectionNum) {
                printf("Relocation section '%s' at offset 0x%3x contains %d entries:\n", RelTabName,relAdd,relTableEntries);
                printf("Offset 	  Info  	type    	Sym.Value     Sym. Name  \n");
                for (int k = 0; k < relTableEntries; k++) {
                    printf("%08x  ", RelocationTable[k].r_offset);
                    int offsetString = ELF32_R_SYM(RelocationTable[k].r_info);
                    int type = ELF32_R_TYPE(RelocationTable[k].r_info);
                    printf("%08x  ", RelocationTable[k].r_info);
                    char *OfficialType = intToType(type);
                    printf("%-10s   ", OfficialType);
                    printf("%08x   ",0);
                    int mark = offsetToString(offsetString);
                    if (mark == 0) {
                        s = getSection(offsetString);
                        printf("%-10s ",s);
                    }
                    printf(" \n");
                }
                printf(" \n");
            }
        }
    }
    char* _startCheck(){
        //check in symtab
        int index=initializeCertainTable(SHT_SYMTAB);
        if(symbolTable) {
            initializeStringTable(SHT_SYMTAB);
            const char *const stringTableOffset = map_start + string_table->sh_offset;
            int symbolTableEntries = mySecHeader[index].sh_size / sizeof(Elf32_Sym);
            for (int j = 0; j < symbolTableEntries; j++) {
                if (strcmp((char *) (stringTableOffset + symbolTable[j].st_name), "_start") == 0)
                    return "SUCCES";
            }
        }
        //check in dynsym
        index=initializeCertainTable(SHT_DYNSYM);
        if(symbolTable) {
            initializeStringTable(SHT_DYNSYM);
            const char *const stringTableOffset1 = map_start + string_table->sh_offset;
            int symbolTableEntries = mySecHeader[index].sh_size / sizeof(Elf32_Sym) ;
            for (int j = 0; j < symbolTableEntries; j++) {
                if (strcmp((char *) (stringTableOffset1 + symbolTable[j].st_name), "_start") == 0)
                    return "SUCCES";
            }
        }
        return "FAILED";
    }

    void Linkcheck(){
        if(!Currentfd){
            perror("FIle wasn't Open\n");
            exit(-1);
        }
        char* status=_startCheck();
        if(strcmp(status, "SUCCES") == 0)
            printf("_start check: PASSED\n");
        else printf("_start check: FAILED\n");
    }

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
        int index=initializeCertainTable(SHT_SYMTAB);
        if(symbolTable) {
            initializeStringTable(SHT_SYMTAB);
            const char *const stringTableOffset = map_start + string_table->sh_offset;

            int symbolTableEntries = mySecHeader[index].sh_size / sizeof(Elf32_Sym);

            if (symbolTableEntries != 0) {
                printf("Symbol table '.symtab' contains %d entries:\n", symbolTableEntries);
                printf("index     value  section_index section_name  symbol_name\n");

                for (int j = 0; j < symbolTableEntries; j++) {
                    printf("[%2d] %8x	\t %-5d	\t SYMTAB  \t \t%-20s\n", j, symbolTable[j].st_value,
                           symbolTable[j].st_shndx, stringTableOffset + symbolTable[j].st_name);
                }
            }
            printf("\n\n");
        }
    }

    void printDYNSYM(){
        int index=initializeCertainTable(SHT_DYNSYM);
        if(symbolTable) {
            initializeStringTable(SHT_DYNSYM);


            const char *const stringTableOffset = map_start + string_table->sh_offset;

            int symbolTableEntries = mySecHeader[index].sh_size / sizeof(Elf32_Sym);
            if (symbolTableEntries != 0) {
                printf("Symbol table '.dynsym' contains %d entries:\n", symbolTableEntries);
                printf("index value   section_index section_name  symbol_name\n");

                for (int j = 0; j < symbolTableEntries; j++) {
                    printf("[%2d] %8x	\t %-5d	\t DYNSYM  \t \t%-20s\n", j, symbolTable[j].st_value,
                           symbolTable[j].st_shndx, stringTableOffset + symbolTable[j].st_name);
                }
            }
            printf("\n\n");
        }
    }

    void PrintSymbols(){
        if(!Currentfd){
            perror("FIle wasn't Open\n");
            exit(-1);
        }
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
        free(fileName);
    }

    printFileDetails(){
        printf("\nFile Details:\n");
        printf("Magic number: %C %C %C \n", myHeader->e_ident[0],myHeader->e_ident[1],myHeader->e_ident[2]);
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
        printf("[Nr]   name \t \t\t   Address  Offset   Size     Type \n");
        Elf32_Shdr *string_table = &mySecHeader[myHeader->e_shstrndx];
        const char *const string_table_of_map = map_start + string_table->sh_offset;//sh_ofsset- begining of file to the first byte in section
        for (int i = 0; i < sectionNum; ++i) {
            char* type =typeToString(mySecHeader[i].sh_type);
            printf("[%2d]: %-20s %08x %08x %08x %-20s \n", i, string_table_of_map + mySecHeader[i].sh_name,mySecHeader[i].sh_addr,mySecHeader[i].sh_offset,mySecHeader[i].sh_size,type);
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


    int main(){

        int bounds=7;
        struct fun_desc menu[] = {{ "Toggle Debug Mode", &ToggleDebugMode }
                ,{ "Examine ELF File", &ExamineELFFile }
                ,{"Print Section Names",&PrintSectionNames}
                ,{"Print Symbols",&PrintSymbols}
                ,{"Link check",&Linkcheck}
                ,{"Relocation Tables - Raw",&RelocationTablesRaw}
                ,{ "Quit", &quit }
                ,{ NULL, NULL } };

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
