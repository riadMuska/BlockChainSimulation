#ifndef _FUNZ_H_
#include "funzioni.h"
#endif

#ifndef _CONF_H_
#include"conf.h"
#endif
#include <stdbool.h>

int utente=0;
int nodo=0;
char *argc[3];
char *argv[3];
int length =0;
char* str;
void creazione_utente(int i)
{
    switch(utente=fork()){
        case -1 :
            printf("Processo utente non creato");
            exit(EXIT_FAILURE);
            break;
        case 0 :
            length = snprintf(NULL, 0, "%d",i);
            str = malloc(length+1);
            snprintf(str,length+1,"%d",i);
            argc[0]=str;
            argc[1]=NULL;
            if(execve("./user",argc,NULL)==-1){
                TEST_ERROR;
                printf("Errore nella chiamata di execve:\n");
            }
            break;
    }
}

void creazione_nodo(int i)
{
    switch(nodo=fork()){
        case -1 :
            printf("Processo nodo non creato");
            exit(EXIT_FAILURE);
            break;

        case 0 :
            length=snprintf(NULL, 0, "%d",i);
            str=malloc(length+1);
            snprintf(str,length+1,"%d",i);
            argv[0] =str;
            if(execve("./node",argv,NULL)==-1){
                TEST_ERROR;
                printf("Errore nella chiamata di execve:\n");
            }
            break;
    }
}

transizione new_transizione(long int time, int sender,  int receiver, int quantita, int reward)
{
    transizione trans;
    trans.timestamp=time;
    trans.sender=sender;
    trans.receiver=receiver;
    trans.quantita=quantita;
    trans.reward=reward;
    return trans;
}

//CREAZIONE LIBRO MASTRO-SHARED MEMORY
int creaLibroMastro(key_t key,size_t size)
{
    int shmid=0;
    shmid=shmget(SHM_KEY ,size,0);
    //*shmat(shmid,NULL,0);
    return 0;
}

bool isFull(transizione *trn)
{
    int i=0;
    bool cond=true;
    for(i=0;i<SO_TP_SIZE && cond;i++)
    {
        if(trn==NULL)
        {
            cond=false;
        }
        else
        {
            cond=true;
        }
        trn++;
    }
    return cond;
}

int randomgen(int min, int max)
{
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    srand(t.tv_nsec);
    return rand() % (max + 1 - min) + min;
}