#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>


void listDirectory(const char * path){
    DIR* dir = NULL;
    struct dirent *entry=NULL;
    char fullPath[512];
    dir=opendir(path);
    if(dir==NULL){
        printf("ERROR\ninvalid directory path\n");
        return;
    }
    
    while((entry=readdir(dir))!=NULL){
        if(strcmp(entry->d_name,".")!=0 && strcmp(entry->d_name,"..")!=0){
            
            snprintf(fullPath,512,"%s/%s",path,entry->d_name);
            printf("%s\n",fullPath);
        }
    }
    
    closedir(dir);
}

void sizeFilter(const char * path, int value){
    DIR* dir = NULL;
    struct dirent *entry=NULL;
    char fullPath[512];
    struct stat buff;
    dir=opendir(path);
    if(dir==NULL){
        printf("ERROR\ninvalid directory path\n");
        return;
    }
    
    while((entry=readdir(dir))!=NULL){
        if(strcmp(entry->d_name,".")!=0 && strcmp(entry->d_name,"..")!=0){ 
            snprintf(fullPath,512,"%s/%s",path,entry->d_name);
            lstat(fullPath,&buff);
            if(S_ISREG(buff.st_mode)){
                if(buff.st_size<value)
                    printf("%s\n",fullPath);
            }      
        }
    }
    
    closedir(dir);
}

void nameFilter(const char * path, char* name_end){
    DIR* dir = NULL;
    struct dirent *entry=NULL;
    char fullPath[512];
    dir=opendir(path);
    if(dir==NULL){
        printf("ERROR\ninvalid directory path\n");
        return;
    }
    
    while((entry=readdir(dir))!=NULL){
        if(strcmp(entry->d_name,".")!=0 && strcmp(entry->d_name,"..")!=0){ 
            snprintf(fullPath,512,"%s/%s",path,entry->d_name);
            int name_size=strlen(entry->d_name);
            int name_end_size=strlen(name_end);
            if(strcmp(name_end,entry->d_name+(name_size-name_end_size))==0)
                printf("%s\n",fullPath);     
        }
    }
    
    closedir(dir);
}

void listDirectoryRec(char *path){
    DIR* dir=NULL;
    struct dirent *entry=NULL;
    char fullPath[512];
    struct stat buff;
    dir=opendir(path);
    if(dir==NULL){
        return;
    }
    while((entry=readdir(dir))!=NULL){
        if(strcmp(entry->d_name,".") && strcmp(entry->d_name,"..")){
            snprintf(fullPath,512,"%s/%s",path,entry->d_name);
            lstat(path,&buff);
            if(S_ISREG(buff.st_mode)){
                printf("%s/%s\n",path,entry->d_name);
            }
            if(S_ISDIR(buff.st_mode)){
                printf("%s/%s\n",path,entry->d_name);
                listDirectoryRec(fullPath);
            }
        }
    }
    closedir(dir);
}

void sizeFilterRec(char *path, int value){
    DIR* dir=NULL;
    struct dirent *entry=NULL;
    char fullPath[512];
    struct stat buff;
    dir=opendir(path);
    if(dir==NULL){
        return;
    }
    while((entry=readdir(dir))!=NULL){
        if(strcmp(entry->d_name,".") && strcmp(entry->d_name,"..")){
            snprintf(fullPath,512,"%s/%s",path,entry->d_name);
            lstat(fullPath,&buff);
            if(S_ISREG(buff.st_mode)){
                if(buff.st_size<value)
                    printf("%s\n",fullPath);
            }
            if(S_ISDIR(buff.st_mode)){
                sizeFilterRec(fullPath,value);
            }
        }
    }
    closedir(dir);
}

void nameFilterRec(char *path, char* name_end){
    DIR* dir=NULL;
    struct dirent *entry=NULL;
    char fullPath[512];
    struct stat buff;
    dir=opendir(path);
    if(dir==NULL){
        return;
    }
    while((entry=readdir(dir))!=NULL){
        if(strcmp(entry->d_name,".") && strcmp(entry->d_name,"..")){
            snprintf(fullPath,512,"%s/%s",path,entry->d_name);
            int name_size=strlen(entry->d_name);
            int name_end_size=strlen(name_end);
            lstat(path,&buff);
            if(S_ISREG(buff.st_mode)){
                if(strcmp(name_end,entry->d_name+(name_size-name_end_size))==0){
                    printf("%s",fullPath);
                }                 
            }
            if(S_ISDIR(buff.st_mode)){
                if(strcmp(name_end,entry->d_name+(name_size-name_end_size))==0){
                    printf("%s",fullPath);
                    nameFilterRec(fullPath,name_end);
                }
            }
        }
    }
    closedir(dir);
}
void list(char* path, int rec){
    printf("SUCCESS\n");
    if(rec==0){
        listDirectory(path);
    }
    else {
        listDirectoryRec(path);
    }
}

int parse(char* path, int validation,int* ok_size){
    int fd1 = open(path,O_RDONLY);
    if(fd1==-1){
        perror("Could not open file");
        return -1;
    }
    char magic[4];
    int header_size=0;
    int version=0;
    int nr_sections=0;
    if(read(fd1,magic,4)==-1){
        perror("Could not read file");
        return -1 ; 
    }
    if(magic[0]!='6'||magic[1]!='L'||magic[2]!='S'||magic[3]!='X'){
        if(validation==0){
            printf("ERROR\nwrong magic\n");
        }
        return -1;
    }
    if(read(fd1,&header_size,2)==-1){
        perror("Could not read file");
        return -1;
    }
    if(read(fd1,&version,4)==-1){
        perror("Could not read file");
        return -1;
    }
    if(version<54 || version>154){
        if(validation==0){
             printf("ERROR\nwrong version\n");
        }
        return -1;
    }
    if(read(fd1,&nr_sections,1)==-1){
        perror("Could not read file");
        return -1;
    }
    if(nr_sections<5 || nr_sections>11){
        if(validation==0){
           printf("ERROR\nwrong sect_nr\n"); 
        }
        return -1;
    }
    for(int i=1;i<=nr_sections;i++){
        char sect_name[13];
        int sect_type=0;
        int sect_offset=0;
        int sect_size=0;
        if(read(fd1,sect_name,13)==-1){
            perror("Could not read file");
            return -1;
        }
        if(read(fd1,&sect_type,4)==-1){
            perror("Could not read file");
            return -1;
        }
        if(sect_type!=84 && sect_type!=63 && sect_type!=43){
            if(validation==0){
                printf("ERROR\nwrong sect_types\n");
            }
            return -1;
        }
        if(read(fd1,&sect_offset,4)==-1){
            perror("Could not read file");
            return -1;
        }
        if(read(fd1,&sect_size,4)==-1){
            perror("Could not read file");
            return -1;
        }
        if(sect_size>1293){
            *ok_size=0;
        }
        
    }
    if(validation==0){
        char s[100];
        snprintf(s,512,"SUCCESS\nversion=%d\nnr_sections=%d\n",version,nr_sections);
        printf("%s",s);
    }
    lseek(fd1,-(nr_sections*25),SEEK_CUR);
    for(int i=1;i<=nr_sections;i++){
        char sect_name[13];
        int sect_type=0;
        int sect_offset=0;
        int sect_size=0;
        if(read(fd1,sect_name,13)==-1){
            perror("Could not read file");
            return -1;
        }
        if(read(fd1,&sect_type,4)==-1){
            perror("Could not read file");
            return -1;
        }
        if(sect_type!=84 && sect_type!=63 && sect_type!=43){
            puts("ERROR\nwrong sect_types");
            return -1;
        }
        if(read(fd1,&sect_offset,4)==-1){
            perror("Could not read file");
            return -1;
        }
        if(read(fd1,&sect_size,4)==-1){
            perror("Could not read file");
            return -1;
        }
    if(validation==0){
        char s[100];
        snprintf(s,512,"section%d: %s %d %d\n",i,sect_name,sect_type,sect_size);
        printf("%s",s);
    }
    }
    return 1;
}

void extract(char* path,int section, int line){
    int fd = open(path,O_RDONLY);
    if(fd==-1){
        printf("ERROR\ninvalide file\n");
        return;
    }
    lseek(fd,10,SEEK_SET);
    int sect_nr=0;
    read(fd,&sect_nr,1);
    char sect_name[13];
        int sect_type=0;
        int sect_offset=0;
        int sect_size=0;
    for(int i=1;i<=section;i++){
        if(read(fd,sect_name,13)==-1){
            perror("Could not read file");
            return;
        }
        if(read(fd,&sect_type,4)==-1){
            perror("Could not read file");
            return;
        }
        if(read(fd,&sect_offset,4)==-1){
            perror("Could not read file");
            return;
        }
        if(read(fd,&sect_size,4)==-1){
            perror("Could not read file");
            return;
        }
    }
    char* buff=(char*)malloc(sect_size*sizeof(char));
    lseek(fd,sect_offset,SEEK_SET);
    read(fd,buff,sect_size);
    int  nr_line=0;
    for(int i=0;i<sect_size;i++){
        if(buff[i]=='\n'|| buff[i]=='\0'){
            nr_line++;
        }
    }
    nr_line++;

    if(line>nr_line){
        printf("ERROR\ninvalide line\n");
    }
    int line_indicator=nr_line;
    char* aux;
    char*  separator="\n";
    aux=strtok(buff,separator);
    while(aux!=NULL){
        if(line_indicator==line){
            break;
        }
        aux=strtok(NULL,separator);
        line_indicator--;
    }
    printf("SUCCESS\n%s\n",aux);
    free(buff);
    close(fd);
}

void findall(char* path){
    DIR* dir=NULL;
    struct dirent *entry=NULL;
    char fullPath[512];
    struct stat buff;
    dir=opendir(path);
    if(dir==NULL){
        return;
    }
    while((entry=readdir(dir))!=NULL){
        if(strcmp(entry->d_name,".") && strcmp(entry->d_name,"..")){
            snprintf(fullPath,512,"%s/%s",path,entry->d_name);
            lstat(fullPath,&buff);
            if(S_ISREG(buff.st_mode)){
                int ok=1;
                if(parse(fullPath,1,&ok)==1){
                    if(ok==1){
                        printf("%s\n",fullPath);
                    }
                }
                
            }
            if(S_ISDIR(buff.st_mode)){
                findall(fullPath);
            }
        }
    }
    closedir(dir);
}

int main(int argc, char **argv){
    if(argc >= 2){
        if(strcmp(argv[1], "variant") == 0){
            printf("55049\n");
        }
        if(strcmp(argv[1],"list")==0){
            if(strstr(argv[2],"path=")){
                list(argv[2]+5,0);

            }
            else if(strcmp(argv[2],"recursive")==0){
                if(strstr(argv[3],"path=")){
                    list(argv[3]+5,1);
                }
                else if(strstr(argv[3],"size_smaller=")){
                    printf("SUCCESS\n");
                    int value;
                    sscanf(argv[3]+13,"%d",&value);
                    sizeFilterRec(argv[4]+5,value);
                }
                else if(strstr(argv[3],"name_ends_with=")){
                    printf("SUCCESS\n");
                    nameFilterRec(argv[4]+5,argv[3]+15);
                }
            }
            else if(strstr(argv[2],"size_smaller=")){
                if(strstr(argv[3],"path=")){
                    printf("SUCCESS\n");
                    int value;
                    sscanf(argv[2]+13,"%d",&value);
                    sizeFilter(argv[3]+5,value);
                }
                else if(strcmp(argv[3],"recursive")==0){
                    printf("SUCCESS\n");
                    int value;
                    sscanf(argv[2]+13,"%d",&value);
                    sizeFilterRec(argv[4]+5,value);
                }
            }
            else if(strstr(argv[2],"name_ends_with=")){
                if(strstr(argv[3],"path=")){
                    printf("SUCCESS\n");
                    nameFilter(argv[3]+5,argv[2]+15);
                }
                else if(strcmp(argv[3],"recursive")==0){
                    printf("SUCCESS\n");
                    nameFilterRec(argv[4]+5,argv[2]+15);
                }
            }       
        }
        if(strcmp(argv[1],"parse")==0){
            if(strstr(argv[2],"path=")){
                int ok=1;
                parse(argv[2]+5,0,&ok);
            }
            else
                perror("Wrong arguments");
        }
        if(strcmp(argv[1],"extract")==0){
            int ok=1;
            if(parse(argv[2]+5,1,&ok)){
                int sect_nr;
                int line_nr;
                sscanf(argv[3]+8,"%d",&sect_nr);
                sscanf(argv[4]+5,"%d",&line_nr);
                extract(argv[2]+5,sect_nr,line_nr);
            }
        }
        if(strcmp(argv[1],"findall")==0){
            printf("SUCCESS\n");
            findall(argv[2]+5);
        }
    }

    return 0;
}