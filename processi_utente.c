#ifndef _FUNZ_H_
#include "funzioni.h"
#endif

#ifndef _CONF_H_
#include"conf.h"
#endif

int retry_time=0,processo_receiver=0,somma_da_inviare=0,somma_da_sottrarre=0,processo_nodo_receiver=0,reward=0,bilancio=0,msgID=0;
struct memCondivisaProcessi *mp;
msgMIO buf;
transizione trn;
struct timespec my_time;
struct libro_mastro *libro_mastro;
int IDSemLm=0;
struct sembuf my_sem_lm;

void handle_signal(int);
void handle_signal_trans(int);

int main(int argc,char *argv[])
{
    int i=0,shmID=0,semID=0,semIDProcessi=0,shmID_processi=0;
    int bilancio=SO_BUDGET_INIT;
    void* shmAddr;
    struct sembuf my_op;
    int semaforo=0;
    int par_i=0;
    struct timespec spec;
    
    struct sigaction sa;
    sigset_t my_mask;

    struct sigaction sa2;
    sigset_t my_mask2;
    int n;
    
    //********** VETTORE PROCESSI UTENTE
    par_i=atoi(argv[0]);
    shmID_processi=shmget(SHM_PROC_KEY,sizeof(*mp),0666);
    mp = shmat(shmID_processi, NULL, 0);
    TEST_ERROR;

    semIDProcessi = semget(SEM_PROC_KEY,0,0666);
    my_op.sem_num = 0;
    my_op.sem_op = -1; 
    semop(semIDProcessi,&my_op,1);

    mp->proc_utente[par_i]=getpid();
    mp->numUsr++;
    n=mp->numUsr;

    my_op.sem_num=0;
    my_op.sem_op=1;
    semop(semIDProcessi,&my_op,1);

    TEST_ERROR;
    //**************** INIZIALIZZAZIONE SEMAFORI
    
    semID = semget(SEM_KEY,0,0666);
    TEST_ERROR;
    if(semID==-1)
    {
        printf("ERRORE NEL RECUPERO DEL SEMAFORO PROCESSI DA PARTE DELLO USER");
        exit(EXIT_FAILURE);
    }
    TEST_ERROR;
    my_op.sem_num = 0;
    my_op.sem_op = -1; 
    semop(semID,&my_op,1);//OGNI PROCESSO DECREMENTA IL SEMAFORO PER FARLO ANDARE A 0
    TEST_ERROR;
    semaforo=semctl(semID,0,GETVAL);
    //printf("VALORE DEL SEMAFORO UTENTE=%d \n",semaforo);

    my_op.sem_num=0;
    my_op.sem_op=0;
    semop(semID,&my_op,1);//BLOCCO L'ESECUZIONE FINCHE SEM=0
    //TEST_ERROR;

    //printf("SONO L'UTENTE: %d!!\n",getpid());
    //**********  INIZIALIZZAZIONE MSG QUEUE
    if((msgID = msgget(MSG_KEY,0600|IPC_NOWAIT))<0)
    {
        printf("msgget error");   
    }

    //**************** INIZIALIZZAZIONE SHARED MEM
    shmID=shmget(SHM_KEY,sizeof(*libro_mastro),0666);
    libro_mastro = shmat(shmID, NULL, 0);
    TEST_ERROR;

    
    IDSemLm=semget(SEM_KEY_LM,0,0600);

    TEST_ERROR;
    if(IDSemLm==-1)
    {
        printf("ERRORE NEL RECUERO DEL SEMAFORO LM DA PARTE DEL NODO.");
        exit(EXIT_FAILURE);
    }

    while(1)
    {

        if(retry_time==SO_RETRY)
        {
            exit(EXIT_SUCCESS);
        }

        bilancio=SO_BUDGET_INIT;
        //////////////////////////////////////HANDLING SIGNAL
        sigemptyset(&my_mask);
        sigaddset(&my_mask, SIGBUS);
        sigprocmask(SIG_BLOCK, &my_mask, NULL);

        bzero(&sa, sizeof(sa));
        sa.sa_handler = handle_signal;
        sigaction(SIGBUS, &sa, NULL);
        //
        sigemptyset(&my_mask2);
        sigaddset(&my_mask2, SIGQUIT);
        sigprocmask(SIG_BLOCK, &my_mask2, NULL);

        bzero(&sa2, sizeof(sa2));
        sa2.sa_handler = handle_signal_trans;
        sigaction(SIGQUIT, &sa2, NULL);
        //////////Ingresso in sezione critica
        my_sem_lm.sem_num=0;
        my_sem_lm.sem_op=-1;
        semop(IDSemLm,&my_sem_lm,1);
        //in sezione critica
        TEST_ERROR;
        for(i=0;i<libro_mastro->cur_index;i++)
        {
            if(libro_mastro->trn[i].receiver==getpid())
            {
                bilancio+=libro_mastro->trn->quantita;
            }
        }
        //uscita sezioone critica
        my_sem_lm.sem_num=0;
        my_sem_lm.sem_op=1;
        semop(IDSemLm,&my_sem_lm,1);
        //sblocco la ricezione dei segnali
        //
        sigaddset(&my_mask2, SIGQUIT);
	    sigprocmask(SIG_UNBLOCK, &my_mask2, NULL);
        sigaddset(&my_mask, SIGBUS);
	    sigprocmask(SIG_UNBLOCK, &my_mask, NULL);
        

        bilancio=bilancio-somma_da_sottrarre;
        if(bilancio>=2)
        {
            do
            {
                processo_receiver =randomgen(0,SO_USERS_NUM-1);
                TEST_ERROR;
            }
            while(mp->proc_utente[processo_receiver]==getpid());

            somma_da_inviare= randomgen(2,bilancio);
            somma_da_sottrarre+=somma_da_inviare;
            reward=(int)(somma_da_inviare/100)*SO_REWARD;
            trn=new_transizione(clock_gettime(CLOCK_REALTIME, &spec),getpid(),mp->proc_utente[processo_receiver],somma_da_inviare,reward);
            processo_nodo_receiver =randomgen(0,SO_NODES_NUM-1);
            buf.trn=trn;
            buf.mtype=mp->proc_nodo[processo_nodo_receiver];
            TEST_ERROR;

            if(msgsnd(msgID,&buf,sizeof(buf)-sizeof(long),0)<0)
            {
                //printf("\nMSGID:%d  sizeof:%ld,ricevitore:%ld\n",msgID,(sizeof(buf)-sizeof(long)),buf.mtype);
                //TEST_ERROR;
                exit(EXIT_FAILURE);
            }
            else
            {
                retry_time=0;
                my_time.tv_nsec = randomgen(SO_MIN_TRANS_GEN_NSEC,SO_MAX_TRANS_GEN_SEC);
                nanosleep (& my_time , NULL );
            }
        }
    }
}

void handle_signal(int signum){
    retry_time++;
}
void handle_signal_trans(int signum)
{
    int i=0;
    struct sigaction sa;
    sigset_t my_mask;

    struct sigaction sa2;
    sigset_t my_mask2;
    bilancio=SO_BUDGET_INIT;
    my_sem_lm.sem_num=0;
    my_sem_lm.sem_op=-1;
    semop(IDSemLm,&my_sem_lm,1);
    TEST_ERROR;
    for(i=0;i<libro_mastro->cur_index;i++)
    {
        if(libro_mastro->trn[i].receiver==getpid())
        {
            bilancio+=libro_mastro->trn->quantita;
        }
    }
    my_sem_lm.sem_num=0;
    my_sem_lm.sem_op=1;
    semop(IDSemLm,&my_sem_lm,1);
    bilancio=bilancio-somma_da_sottrarre;
    if(bilancio>=2)
    {
        do
        {
            processo_receiver =randomgen(0,SO_USERS_NUM-1);
            TEST_ERROR;
        }
        while(mp->proc_utente[processo_receiver]==getpid());

        somma_da_inviare= randomgen(2,bilancio);
        somma_da_sottrarre+=somma_da_inviare;
        reward=(int)(somma_da_inviare/100)*SO_REWARD;
        trn=new_transizione(300,getpid(),mp->proc_utente[processo_receiver],somma_da_inviare,reward);
        processo_nodo_receiver =randomgen(0,SO_NODES_NUM-1);
        buf.trn=trn;
        buf.mtype=mp->proc_nodo[processo_nodo_receiver];
        TEST_ERROR;

        if(msgsnd(msgID,&buf,sizeof(buf)-sizeof(long),0)<0)
        {
            exit(EXIT_FAILURE);
        }
        else
        {
            retry_time=0;
            my_time.tv_nsec = randomgen(SO_MIN_TRANS_GEN_NSEC,SO_MAX_TRANS_GEN_SEC);
            nanosleep (& my_time , NULL );
        }
    }
}