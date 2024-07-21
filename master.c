#ifndef _FUNZ_H_
#include "funzioni.h"
#endif

#ifndef _CONF_H_
#include"conf.h"
#endif

void sigchld_handler(int );
void sig_mastro_full_handlert(int);
void arresto(int);
int current_proc=SO_USERS_NUM;
struct libro_mastro *libro_mastro;
struct memCondivisaProcessi *mp;
int main()
{
    int i=0;
    int semID=0;
    int semaforo=3;
    int shmID=0;
    int shmID_processi=0;
    int msgID=0;
    int semLM=0;
    int semPro=0;
    struct sigaction sa;

    signal(SIGALRM,sig_handler);
    alarm(SO_SIM_SECONDI);

    signal(SIGCHLD,sigchld_handler);

    signal(SIGUSR1,sig_mastro_full_handlert);

    //*************** CREZIONE SEMAFORI WAIT FOR 0 PER LA SINCRONIZZAZIONE DEI PROCESSI
    
    semID = semget(SEM_KEY, 1, IPC_CREAT| 0666);
    TEST_ERROR;
    if(semID==-1)
    {
        printf("\n\nimpossibile creare il semaforo UTENTE\n\n");
        exit(EXIT_FAILURE);
    }
    semctl(semID, 0, SETVAL, SO_NODES_NUM+SO_USERS_NUM);
    TEST_ERROR;
    
    // CREZIONE MEMORIA CONDIVISA PER I PROCESSI, SERVE A PASSARSI I PID
    shmID_processi=shmget(SHM_PROC_KEY,sizeof(*mp),IPC_CREAT|0600);
    if(shmID_processi==-1)
    {
        TEST_ERROR;
        printf("ERRORE NELLA CREAZIONE DELLA SHMPROCESSI");
        exit(EXIT_FAILURE);
    }
    

    //***************** CREAAZIONE LIBRO MASTRO
    shmID=shmget(SHM_KEY,sizeof(*libro_mastro),IPC_CREAT|0600);
    if(shmID==-1)
    {
        TEST_ERROR;
        printf("ERRORE NELLA CREAZIONE DEL LIBRO MASTRO");
        exit(EXIT_FAILURE);
    }
    libro_mastro=shmat(shmID,NULL,0);
    TEST_ERROR;
    libro_mastro->cur_index=0;
    TEST_ERROR;

    //******************* CREAZIONE DEL SEMAFORO PER ACCEDERE AL LIBRO MASTRO
    semLM = semget(SEM_KEY_LM, 1, IPC_CREAT| 0666);
    TEST_ERROR;
    if(semLM==-1)
    {
        printf("\n\nimpossibile creare il semaforo PER ACCEDERE AL LIBROMASTRO\n\n");
        exit(EXIT_FAILURE);
    }
    semctl(semLM, 0, SETVAL, 1);
    TEST_ERROR;
    //******************* CREAZIONE DEL SEMAFORO PER ACCEDERE ALLLA MEMORIA CONDIVISA FRA PROCESSI 
    semPro = semget(SEM_PROC_KEY, 1, IPC_CREAT| 0666);
    TEST_ERROR;
    if(semPro==-1)
    {
        printf("\n\nimpossibile creare il semaforo PER ACCEDERE AL LIBROMASTRO\n\n");
        exit(EXIT_FAILURE);
    }
    semctl(semPro, 0, SETVAL, 1);
    TEST_ERROR;
    //***********************  CREAZIONE MSG QUEUE PER COMUNICAZIONE NODI UTENTI 
    if((msgID = msgget(MSG_KEY, IPC_CREAT | 0600))<0)
    {
        printf("msgget error");
        TEST_ERROR
    }
    //CREAZIONE PROCESSI UTENTE
    for(i=0;i<SO_USERS_NUM;i++)
    {
        creazione_utente(i);
    }
    //CREAZIONE PROCESSI NODO
    for(i=0;i<SO_NODES_NUM;i++)
    {
        creazione_nodo(i);
    }

    mp=shmat(shmID_processi,NULL,0);
    TEST_ERROR;
    while(1)
    {
        int budgets_utenti[SO_USERS_NUM];
        int budgets_nodi[SO_NODES_NUM];
        printf("\nNUMERO PROCESSI UTENTE ATTIVI: %d",current_proc);
        printf("\nNUMERO PROCESSI NODO ATTIVI: %d",SO_NODES_NUM);
        for(int y=0;y<SO_USERS_NUM;y++)
        {
            budgets_utenti[y]=SO_BUDGET_INIT;
            for(int g=0;g<libro_mastro->cur_index;g++)
            {
                if(libro_mastro->trn[g].sender==mp->proc_utente[y])
                {
                    budgets_utenti[y]-=libro_mastro->trn[g].quantita;
                }
                if(libro_mastro->trn[g].receiver==mp->proc_utente[y])
                {
                    budgets_utenti[y]+=libro_mastro->trn[g].quantita;
                }
            }
            for(int y=0;y<SO_NODES_NUM;y++)
            {
                budgets_nodi[y]=0;
                for(int g=0;g<libro_mastro->cur_index;g++)
                {
                    if(libro_mastro->trn[g].receiver==mp->proc_nodo[y])
                    {
                        budgets_nodi[y]+=libro_mastro->trn[g].quantita;
                    }
                }
            }
        }
        if(SO_USERS_NUM+SO_NODES_NUM<50)
        {
            for(int y=0;y<SO_USERS_NUM;y++)
            {
                printf("\nBUDGET PROCESSO UTENTE %d---->%d",mp->proc_utente[y],budgets_utenti[y]);
            }

            for(int y=0;y<SO_NODES_NUM;y++)
            {
                printf("\nBUDGET PROCESSO NODO %d---->%d",mp->proc_nodo[y],budgets_nodi[y]);
            }
            printf("\n NUM UTENTI TERMINATI PREMATURAMENTE: %d\n",SO_USERS_NUM-current_proc);
            sleep(1);
        }
        else
        {
            int max=0;
            int min=0;
            int min_index=0;
            int max_index=0;
            int ut_min=0;
            int ut_max=0;
            for(int y=0;y<SO_USERS_NUM;y++)
            {
                if(budgets_utenti[y]<min)
                {
                    min=budgets_utenti[y];
                    min_index=y;
                    ut_min=1;
                }
                if(max<budgets_utenti[y])
                {
                    max=budgets_utenti[y];
                    ut_max=1;
                    max_index=y;
                }
            }

            for(int y=0;y<SO_NODES_NUM;y++)
            {
                if(budgets_nodi[y]<min)
                {
                    min=budgets_nodi[y];
                    ut_min=0;
                    min_index=y;
                }
                if(max<budgets_nodi[y])
                {
                    max=budgets_nodi[y];
                    ut_max=0;
                    min_index=y;
                }
            }
            if(ut_min==1)
            {
                printf("\nBUDGET PROCESSO UTENTE MIN %d---->%d",mp->proc_utente[min_index],budgets_utenti[min_index]);
            }
            else
            {
                printf("\nBUDGET PROCESSO NODO MIN %d---->%d",mp->proc_nodo[min_index],budgets_nodi[min_index]);
            }
            if(ut_max==1)
            {
                printf("\nBUDGET PROCESSO UTENTE MAX %d---->%d",mp->proc_utente[max_index],budgets_utenti[max_index]);
            }
            else
            {
                printf("\nBUDGET PROCESSO NODO MAX %d---->%d",mp->proc_nodo[max_index],budgets_nodi[max_index]);
            }

        }
    }
}
void sigchld_handler(int signum){//uno user è terminato
    current_proc--;//aggiorno numero di processi utente attivi
    if(current_proc==0)
    {
        arresto(2);
    }
}
void arresto(int modo)
{
    int budgets_utenti[SO_USERS_NUM];
    int budgets_nodi[SO_NODES_NUM];
    //printf("\nTRRANSAZIONI SCRITTE %d, NUM TRANS CONTENTI LM %d\n",libro_mastro->cur_index,(SO_BLOCK_SIZE*SO_REGISTRY_SIZE));
    switch(modo)
    {
        case 0: printf("\nTERMINO PERHCÈ PASSATI SO_SIM_SECONDI\n");break;
        case 1: printf("\nTERMINO PERCHÈ LIBRO MASTRO HA RAGGIUNTO LA DIMENSIONE MASSIMA\n");break;
        case 2:printf("\nTERMINO PERCHÈ NON CI SONO PIU PROCESSI ATTIVI\n");break;
        default:printf("\nTERMINO PER MOTIVI ANOMALI");break;
    }

    printf("\nNUM BLOCCHI SCRITTI: %d\n",(libro_mastro->cur_index/SO_BLOCK_SIZE));
    //for(int i=0;i<libro_mastro->cur_index;i++)
    //{
        //printf("\ntransazione i: %d  timestamp(%d) sender(%d) receivere(%d) reward(%d) quantita(%d)\n",i,libro_mastro->trn[i].timestamp,libro_mastro->trn[i].sender,libro_mastro->trn[i].receiver,libro_mastro->trn[i].reward,libro_mastro->trn[i].quantita);
    //}
    for(int y=0;y<SO_USERS_NUM;y++)
    {
        budgets_utenti[y]=SO_BUDGET_INIT;
        for(int g=0;g<libro_mastro->cur_index;g++)
        {
            if(libro_mastro->trn[g].sender==mp->proc_utente[y])
            {
                budgets_utenti[y]-=libro_mastro->trn[g].quantita;
            }
            if(libro_mastro->trn[g].receiver==mp->proc_utente[y])
            {
                budgets_utenti[y]+=libro_mastro->trn[g].quantita;
            }
        }
        printf("\nBUDGET PROCESSO UTENTE %d---->%d",mp->proc_utente[y],budgets_utenti[y]);
    }

    for(int y=0;y<SO_NODES_NUM;y++)
    {
        budgets_nodi[y]=0;
        for(int g=0;g<libro_mastro->cur_index;g++)
        {
            if(libro_mastro->trn[g].receiver==mp->proc_nodo[y])
            {
                budgets_nodi[y]+=libro_mastro->trn[g].quantita;
            }
        }
        printf("\nBUDGET PROCESSO NODO %d---->%d\n",mp->proc_nodo[y],budgets_nodi[y]);
    }
    printf("\nNUM UTENTI TERMINATI PREMATURAMENTE: %d\n",SO_USERS_NUM-current_proc);

    for(int y=0;y<SO_NODES_NUM;y++)
    {
        kill(mp->proc_utente[y],SIGINT);
    }

    for(int y=0;y<SO_NODES_NUM;y++)
    {
        kill(mp->proc_nodo[y],SIGUSR1);
    }

    exit(EXIT_SUCCESS);
}
void sig_handler(int signum)
{
    arresto(0);
}
void sig_mastro_full_handlert(int signum)
{
    arresto(1);
}