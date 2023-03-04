#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <stdlib.h>
#define FIFO_WRITE "RESP_PIPE_55049"
#define FIFO_READ "REQ_PIPE_55049"

int main(){
    int shmFd = 0;
    int fd1 = -1;
    int fd2 = -1;
    int map_file_fd = -1;
    int file_size = 0;
    char* sharedInt  = NULL;
    char* data = NULL;
    unlink(FIFO_WRITE);
    if(mkfifo(FIFO_WRITE,0600)!=0){
        perror("ERROR\ncannot create the response pipe");
        return 1;
    }
    fd2 = open(FIFO_READ,O_RDONLY);
    if(fd2==-1){
        perror("ERROR\ncannot open the request pipe");
        return 1;
    }
    fd1 = open(FIFO_WRITE,O_WRONLY);
    if(fd1==-1){
        perror("ERROR\ncannot open the response pipe");
        return 1;
    }
    char connect[] = "CONNECT";
    char size_connect =(char)strlen(connect);
    write(fd1,&size_connect,1);
    write(fd1,connect,(int)size_connect);
    printf("SUCCESS\n");
    char error[] = "ERROR";
    //char error2[] = "Error";
    char error_size = 5;
    char success[] = "SUCCESS";
    char success_size = (char)strlen(success);
    for(;;){
        //puts("OKK");
        int size = 0;
        read(fd2,&size,1);
        char *request =(char*)malloc(size*sizeof(char));
        read(fd2,request,size);
        request[size]='\0';
        //puts(request);
        if(strcmp(request,"PING")==0){
            char pong[] = "PONG";
            char size_pong = 4;
            write(fd1,&size,1);
            write(fd1,request,size);
            write(fd1,&size_pong,1);
            write(fd1,pong,size_pong);
            unsigned int number = 55049;
            write(fd1,&number,sizeof(unsigned int));
        }
        if(strcmp(request,"CREATE_SHM")==0){
            unsigned int mem_size = 0;
            read(fd2,&mem_size,sizeof(unsigned int));
            shmFd = shm_open("/Kx8iTZ4P",O_CREAT | O_RDWR,0664);
            if(shmFd<0){
                write(fd1,&size,1);
                write(fd1,request,size);
                write(fd1,&error_size,1);
                write(fd1,error,error_size);
                return 1;
            }
            ftruncate(shmFd,mem_size);
            sharedInt = (char*)mmap(0,mem_size,PROT_READ | PROT_WRITE,MAP_SHARED,shmFd,0);
            if(sharedInt==(void*)-1){
                write(fd1,&size,1);
                write(fd1,request,size);
                write(fd1,&error_size,1);
                write(fd1,error,error_size);
                return 1;
            }
            write(fd1,&size,1);
            write(fd1,request,size);
            write(fd1,&success_size,1);
            write(fd1,success,success_size);
        }
        if(strcmp(request,"WRITE_TO_SHM")==0){
            unsigned int offset = 0;
            unsigned int value = 0;
            read(fd2,&offset,sizeof(unsigned int));
            read(fd2,&value,sizeof(unsigned int));
            if(offset<0 || offset>3166902 || (offset+sizeof(value))<0 || (offset+sizeof(value))>3166902 ){
                write(fd1,&size,1);
                write(fd1,request,size);
                write(fd1,&error_size,1);
                write(fd1,error,error_size);
                return 1;
            }
            memcpy(sharedInt+offset,&value,sizeof(unsigned int));
            write(fd1,&size,1);
            write(fd1,request,size);
            write(fd1,&success_size,1);
            write(fd1,success,success_size);
        }
        if(strcmp(request,"MAP_FILE")==0){
            int size_fileName = 0;
            read(fd2,&size_fileName,1);
            char* fileName = (char*)malloc(size_fileName*sizeof(char));
            read(fd2,fileName,size_fileName);
            fileName[size_fileName]='\0';
            map_file_fd = open(fileName,O_RDONLY);
            if(map_file_fd==-1){
                write(fd1,&size,1);
                write(fd1,request,size);
                write(fd1,&error_size,1);
                write(fd1,error,error_size);
                free(fileName);
                return 1;
            }
            file_size = lseek(map_file_fd,0,SEEK_END);
            lseek(map_file_fd,0,SEEK_SET);
            data = (char*)mmap(NULL,file_size,PROT_READ,MAP_PRIVATE,map_file_fd,0);
            if(data==(void*)-1){
                write(fd1,&size,1);
                write(fd1,request,size);
                write(fd1,&error_size,1);
                write(fd1,error,error_size);
                free(fileName);
                close(map_file_fd);
                return 1;
            }
            write(fd1,&size,1);
            write(fd1,request,size);
            write(fd1,&success_size,1);
            write(fd1,success,success_size);
            free(fileName);
        }
        if(strcmp(request,"READ_FROM_FILE_OFFSET")==0){
            unsigned int offset = 0;
            unsigned int noBytes = 0;
            read(fd2,&offset,sizeof(unsigned int));
            read(fd2,&noBytes,sizeof(unsigned int));
            printf("offset = %d noBytes=%d\n",offset,noBytes);
            if(data==NULL || map_file_fd==-1 || (offset+noBytes)>file_size){
                write(fd1,&size,1);
                write(fd1,request,size);
                write(fd1,&error_size,1);
                write(fd1,error,error_size);
            }
            else{
                int j=0;
                for(int i=offset;i<(offset+noBytes);i++){
                    sharedInt[j]=data[i];
                    j++;
                }
                write(fd1,&size,1);
                write(fd1,request,size);
                write(fd1,&success_size,1);
                write(fd1,success,success_size);
            }
        }
        if(strcmp(request,"READ_FROM_FILE_SECTION")==0){
            unsigned int noSection = 0;
            unsigned int offset = 0;
            unsigned int noBytes = 0;
            read(fd2,&noSection,sizeof(unsigned int));
            read(fd2,&offset,sizeof(unsigned int));
            read(fd2,&noBytes,sizeof(unsigned int));
            char numberOfSections = data[10];
            if((int)numberOfSections<noSection){
                write(fd1,&size,1);
                write(fd1,request,size);
                write(fd1,&error_size,1);
                write(fd1,error,error_size);
            }
            else{
                int index = 11 + 25*(noSection-1)+17;
                int sectionOffset = 0;
                memcpy(&sectionOffset,&data[index],4);
                int j=0;
                for(int i=sectionOffset+offset;i<(offset+noBytes+sectionOffset);i++){
                    sharedInt[j]=data[i];
                    j++;
                }
                write(fd1,&size,1);
                write(fd1,request,size);
                write(fd1,&success_size,1);
                write(fd1,success,success_size);
            }
        }
        if(strcmp(request,"READ_FROM_LOGICAL_SPACE_OFFSET")==0){
            return 1;
        }
        if(strcmp(request,"EXIT")==0){
            munmap(data,file_size);
            close(map_file_fd);
            munmap((void*)sharedInt,3166902);
            sharedInt = NULL;
            close(shmFd);
            shm_unlink("/Kx8iTZ4P");
            close(fd2);
            close(fd1);
            unlink(FIFO_WRITE);
            break;
            free(request);
        }
    }

    return 0;
}