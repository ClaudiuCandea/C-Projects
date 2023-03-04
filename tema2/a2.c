#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "a2_helper.h"
sem_t sem1;
sem_t sem2;
sem_t sem3;
sem_t barrier1;
sem_t barrier2;
int count = 0;
int isActiv = 0;
int isDone = 0;
typedef struct{
    int id;
    sem_t *sem1;
    sem_t *sem2;
}THREAD_STRUCT;

typedef struct{
    int id;
    sem_t *sem1;
    sem_t *sem2;
    sem_t *sem3;
    sem_t *barrier1;
    sem_t *barrier2;
}THREAD_STRUCT2;

sem_t *sem4=NULL;
sem_t *sem5=NULL;

void *thread_function(void *param)
{   THREAD_STRUCT *s =(THREAD_STRUCT*)param;
    if(s->id==3){
        sem_wait(s->sem1);
    }
    if(s->id==4){
        sem_wait(sem4);
    }
    info(BEGIN, 5, s->id);
    if(s->id==5){
        sem_post(s->sem1);
        sem_wait(s->sem2);
    }
    info(END, 5, s->id);
    if(s->id==4){
        sem_post(sem5);
    }
    if(s->id==3){
        sem_post(s->sem2);
    }
    return NULL;
}
void *thread_function3(void *param)
{   THREAD_STRUCT *s =(THREAD_STRUCT*)param;
    if(s->id == 5){
        sem_wait(sem5);
    }
    info(BEGIN, 6, s->id);
    info(END, 6, s->id);
    if(s->id == 4){
        sem_post(sem4);
    }
    return NULL;
}
void *thread_function2(void *param)
{   THREAD_STRUCT2 *s =(THREAD_STRUCT2*)param;
    if(s->id == 12){
        isActiv = 1;
    }
    else{
        if(isActiv == 0){
            sem_wait(s->barrier1);
        }
    }
    sem_wait(s->sem1);
    info(BEGIN, 7, s->id);
    sem_post(s->barrier1);
    sem_wait(s->sem2);
    count++;
    sem_post(s->sem2);
    if(count == 5){
        sem_post(s->barrier2);
    }
    if(s->id == 12){
        while(count!=5){
        sem_wait(s->barrier2);
        }
        isDone = 1;
    }
    if((isActiv==1)&&(isDone==0)){
        sem_wait(s->sem3);
    }
    sem_wait(s->sem2);
    count--;
    sem_post(s->sem2);
    info(END, 7, s->id);
    if(s->id != 12){
        sem_post(s->sem3);
    }
    sem_post(s->sem1);
    return NULL;
}

int main(){
    init();
    sem4 = sem_open("sem4",O_CREAT,0644,0);
    sem5 = sem_open("sem5",O_CREAT,0644,0);
    info(BEGIN, 1, 0);
    pid_t pid2=-1, pid3=-1, pid4=-1, pid5=-1, pid6=-1, pid7=-1;
    pid2 = fork();
    if(pid2 == -1){
        perror("Could not create child process p2");
        return -1;
    }
    if(pid2 == 0){
        info(BEGIN, 2, 0);
        pid6 = fork();
        if(pid6 == -1){
            perror("Could not create child process p6");
            return -1;
        }
        if(pid6==0){
            info(BEGIN, 6, 0);
            pthread_t tid[5];
                sem_init(&sem1,0,0);
                sem_init(&sem2,0,0);
                THREAD_STRUCT param[5];
                for(int i=0;i<5;i++){
                    param[i].sem1=&sem1;
                    param[i].sem2=&sem2;
                    param[i].id=i+1;
                    pthread_create(&tid[i],NULL,thread_function3,&param[i]);
                }
                for(int i=0;i<5;i++){
                    pthread_join(tid[i],NULL);
                }
                sem_destroy(&sem1);
                sem_destroy(&sem2);
                info(END, 6, 0);
        }
        else{
            waitpid(pid6,NULL,0);
            info(END, 2, 0);
        }
    }
    else{
        pid3 = fork();
        if(pid3 == -1){
            perror("Could not create child process p3");
            return -1;
        }
        if(pid3 == 0){
            info(BEGIN, 3, 0);
            pid5 = fork();
            if(pid5 == -1){
                perror("Could not create child process p6");
                return -1;
            }
            if(pid5==0){
                info(BEGIN, 5, 0);
                pthread_t tid[5];
                sem_init(&sem1,0,0);
                sem_init(&sem2,0,0);
                THREAD_STRUCT param[5];
                for(int i=0;i<5;i++){
                    param[i].sem1=&sem1;
                    param[i].sem2=&sem2;
                    param[i].id=i+1;
                    pthread_create(&tid[i],NULL,thread_function,&param[i]);
                }
                for(int i=0;i<5;i++){
                    pthread_join(tid[i],NULL);
                }
                sem_destroy(&sem1);
                sem_destroy(&sem2);
                info(END, 5, 0);
            }
            else{
                waitpid(pid5,NULL,0);
                info(END, 3, 0);
            }
        }
        else{
            pid4 = fork();
            if(pid4 == -1){
                perror("Could not create child process p4");
                return -1;
            }
            if(pid4 == 0){
                info(BEGIN, 4, 0);
                pid7 = fork();
                if(pid7 == -1){
                    perror("Could not create child process p6");
                    return -1;
                }
                if(pid7==0){
                    info(BEGIN, 7, 0);
                    pthread_t tid[37];
                    sem_init(&sem1,0,5);
                    sem_init(&sem2,0,1);
                    sem_init(&barrier1,0,0);
                    sem_init(&barrier2, 0, 0);
                    sem_init(&sem3, 0 ,0);
                    THREAD_STRUCT2 param[37];
                    for(int i=0;i<37;i++){
                        param[i].sem1=&sem1;
                        param[i].sem2=&sem2;
                        param[i].sem3=&sem3;
                        param[i].barrier1=&barrier1;
                        param[i].barrier2=&barrier2;
                        param[i].id=i+1;
                        pthread_create(&tid[i],NULL,thread_function2,&param[i]);
                    }
                    for(int i=0;i<37;i++){
                        pthread_join(tid[i],NULL);
                    }
                    sem_destroy(&sem1);
                    sem_destroy(&sem2);
                    sem_destroy(&sem3);
                    sem_destroy(&barrier1);
                    sem_destroy(&barrier2);
                    info(END, 7, 0);
                }
                else{
                    waitpid(pid7,NULL,0);
                    info(END, 4, 0);
                }
            }
            else{
                waitpid(pid2,NULL,0);
                waitpid(pid3,NULL,0);
                waitpid(pid4,NULL,0);
                info(END, 1, 0);
                return 0;
            }
        }
    }
sem_close(sem4);
sem_close(sem5);
}
