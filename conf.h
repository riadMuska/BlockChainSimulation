#ifndef _CONF_H_
#define _CONF_H_

//#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <signal.h>
#include <time.h>
#include <sys/msg.h>

#define MSG_MAX_SIZE     128
#define TEST_ERROR    if (errno) {dprintf(STDERR_FILENO, \
					   "%s:%d: PID=%5d: Error %d (%s)\n",\
					   __FILE__,\
					   __LINE__,\
					   getpid(),\
					   errno,\
					   strerror(errno));}

//budget di ogni processo utente all'inizio alla creazione
#define SO_SIM_SECONDI 30
#define SO_USERS_NUM 9
#define SO_NODES_NUM 3
#define SO_BUDGET_INIT 5000
#define SO_MIN_TRANS_GEN_NSEC 100000000
#define SO_MAX_TRANS_GEN_SEC  200000000
#define SO_RETRY 3
#define SO_TP_SIZE 5
#define SO_BLOCK_SIZE 3
#define SO_REGISTRY_SIZE 4
#define SO_REWARD 20
#define REWARD_SENDER -1
//KEYS OGGETTI ICP
#define MSG_KEY 12562
#define SHM_KEY 32123
#define SEM_KEY 45673
#define SEM_KEY_LM 98765
#define SHM_PROC_KEY 43566
#define SEM_PROC_KEY 76849

typedef  struct
{
    int timestamp;//tempo in nanosec della transizione
    int sender;//implicito, id dell'utente che manda la tranzione
    int receiver;//id dell'utente a cui arrivano i soldi
    int quantita;//quantità di denro inviata
    int reward;//quantità che verrà trattenuta per l'utilizzo del servizio
}transizione;

struct node {
   transizione trn;
   struct node *next;
};

//typedef nodi *lista_trn;
//lista_trn transaction_pool=NULL;
//lista_trn last;


typedef struct {
  long mtype;    
  transizione trn;
}msgMIO;


struct memCondivisaProcessi
{
    unsigned long cur_index;
    int proc_utente[SO_USERS_NUM];
    int proc_nodo[SO_NODES_NUM];
    int pidMater;
    int numUsr;
};

struct semMIO {
  unsigned short sem_num;
  short sem_op;
  short sem_flg;
};

struct libro_mastro
{
    int cur_index;
    transizione trn[(SO_BLOCK_SIZE*SO_REGISTRY_SIZE)];
};
#endif