#ifndef _FUNZ_H_
#include "funzioni.h"
#endif

#ifndef _CONF_H_
#include"conf.h"
#endif

void sig_handler(int);
int indice_ultimo_elemento_tp=-1;

int main(int argc,char *argv[])
{
    transizione blocco_candidato[SO_BLOCK_SIZE];
    transizione transizione_reward;
    transizione transaction_pool[SO_TP_SIZE];
    struct timespec my_time;
    struct timespec inizio;
    struct libro_mastro *libro_mastro;
    struct memCondivisaProcessi *mp;
    struct sembuf my_op;
    struct sembuf my_sem_lm;
    msgMIO msg;
    int i=0,m_id=0, somma_reward=0,semID=0, semaforo=0, shmID_processi=0, par_i=0, IDlibroMatro=0,IDSemLm=0;
    int z=0;
    int semIDProcessi=0;

    par_i=atoi(argv[0]);
    shmID_processi=shmget(SHM_PROC_KEY,sizeof(*mp),0666);
    mp = shmat(shmID_processi, NULL, 0);

    semIDProcessi = semget(SEM_PROC_KEY,0,0666);
    my_op.sem_num = 0;
    my_op.sem_op = -1; 
    semop(semIDProcessi,&my_op,1);
    
    TEST_ERROR;
    mp->proc_nodo[par_i]=getpid();

    my_op.sem_num=0;
    my_op.sem_op=1;
    semop(semIDProcessi,&my_op,1);


    
    //**************** INIZIALIZZAZIONE SEMAFORI
    semID = semget(SEM_KEY,0,0666);
    TEST_ERROR;
    if(semID==-1)
    {
        printf("ERRORE NEL RECUPERO DEL SEMAFORO PROCESSI DA PARTE DELLO USER");
        exit(EXIT_FAILURE);
    }
    my_op.sem_num = 0;
    my_op.sem_op = -1; 
    semop(semID,&my_op,1);//OGNI PROCESSO DECREMENTA IL SEMAFORO PER FARLO ANDARE A 0
    TEST_ERROR;
    semaforo=semctl(semID,0,GETVAL);
    //printf("VALORE DEL SEMAFORO NODO=%d \n",semaforo);

    my_op.sem_num=0;
    my_op.sem_op=0;
    semop(semID,&my_op,1);//BLOCCO L'ESECUZIONE FINCHE SEM=0
    //TEST_ERROR;

    //signal(SIGINT,sigterm_handler);
    signal(SIGUSR1,sig_handler);

    //printf("SONO IL NODO: %d!!\n",getpid());
    if((m_id = msgget(MSG_KEY, 0600))<0)
    {
        printf("msgget error");   
    }

    i=0;
    IDlibroMatro=shmget(SHM_KEY,sizeof(*libro_mastro),0600);
    TEST_ERROR;
    libro_mastro = shmat(IDlibroMatro, NULL, 0);
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
        if(msgrcv(m_id,&msg,sizeof(msg)-sizeof(long),getpid(),0)==-1)
        {
            TEST_ERROR;
        }
        
        else
        {
            if(indice_ultimo_elemento_tp<SO_BLOCK_SIZE-1 && libro_mastro->cur_index<(SO_REGISTRY_SIZE*SO_BLOCK_SIZE)-1)
            {
                indice_ultimo_elemento_tp++;
                transaction_pool[indice_ultimo_elemento_tp].quantita=msg.trn.quantita;
                transaction_pool[indice_ultimo_elemento_tp].sender=msg.trn.sender;
                transaction_pool[indice_ultimo_elemento_tp].receiver=msg.trn.receiver;
                transaction_pool[indice_ultimo_elemento_tp].reward=msg.trn.reward;
                transaction_pool[indice_ultimo_elemento_tp].timestamp=msg.trn.timestamp;
                TEST_ERROR;
                //printf("\nTPOOL[]: %d,%d----ultimo elemtento:%d",transaction_pool[indice_ultimo_elemento_tp].sender,transaction_pool[indice_ultimo_elemento_tp].quantita,indice_ultimo_elemento_tp);
                TEST_ERROR;
                if(indice_ultimo_elemento_tp>=SO_BLOCK_SIZE-1 /*&& (libro_mastro->cur_index<((SO_REGISTRY_SIZE*SO_BLOCK_SIZE)-1))*/)
                {

                    for(i=0;i<SO_BLOCK_SIZE-1;i++)
                    {
                        blocco_candidato[i].sender=transaction_pool[i].sender;
                        blocco_candidato[i].receiver=transaction_pool[i].receiver;
                        blocco_candidato[i].quantita=transaction_pool[i].quantita;
                        blocco_candidato[i].reward=transaction_pool[i].reward;
                        blocco_candidato[i].timestamp=transaction_pool[i].timestamp;
                        somma_reward+=transaction_pool[i].reward;
                        //printf("\n nSONO IL NODO %dTRANSAZIONI CHE STO PER SCRIVERE NEL BLOCCO: i: sender(%d) receiver(%d) quantità(%d) reward(%d) timestamp(%d)",getpid(),blocco_candidato[i].sender,blocco_candidato[i].receiver,blocco_candidato[i].quantita,blocco_candidato[i].reward,blocco_candidato[i].timestamp);
                        //sleep(2);
                    }
                    blocco_candidato[SO_BLOCK_SIZE-1].timestamp=200;
                    blocco_candidato[SO_BLOCK_SIZE-1].sender=REWARD_SENDER;
                    blocco_candidato[SO_BLOCK_SIZE-1].receiver=getpid();
                    blocco_candidato[SO_BLOCK_SIZE-1].quantita=somma_reward;
                    blocco_candidato[SO_BLOCK_SIZE-1].reward=0;
                    my_time.tv_nsec = randomgen(SO_MIN_TRANS_GEN_NSEC,SO_MAX_TRANS_GEN_SEC);
                    my_time.tv_sec=1;
                    nanosleep(&my_time,NULL);
                    TEST_ERROR;
                    my_sem_lm.sem_num=0;
                    my_sem_lm.sem_op=-1;
                    semop(IDSemLm,&my_sem_lm,1);
                    TEST_ERROR;


                    for(int i=0;i<SO_BLOCK_SIZE;i++)
                    {
                        //printf("\n\t CUR INDEX: %d",libro_mastro->cur_index+i);
                        libro_mastro->trn[libro_mastro->cur_index+i].quantita=blocco_candidato[i].quantita;
                        libro_mastro->trn[libro_mastro->cur_index+i].sender=blocco_candidato[i].sender;
                        libro_mastro->trn[libro_mastro->cur_index+i].receiver=blocco_candidato[i].receiver;
                        libro_mastro->trn[libro_mastro->cur_index+i].reward=blocco_candidato[i].reward;
                        libro_mastro->trn[libro_mastro->cur_index+i].timestamp=blocco_candidato[i].timestamp;
                        //printf("\n nSONO IL NODO %dTRANSAZIONI CHE STO PER SCRIVERE NEL BLOCCO: i: sender(%d) receiver(%d) quantità(%d) reward(%d) timestamp(%d)",getpid(),blocco_candidato[i].sender,blocco_candidato[i].receiver,blocco_candidato[i].quantita,blocco_candidato[i].reward,blocco_candidato[i].timestamp);
                        //printf("\nSONO IL NODO %d  TRANSAZIONI CHE STO PER SCRIVERE NEL BLOCCO: i: sender(%d) receiver(%d) quantità(%d) reward(%d) timestamp(%d)",getpid(),libro_mastro->trn[libro_mastro->cur_index+i].sender,libro_mastro->trn[libro_mastro->cur_index+i].receiver,libro_mastro->trn[libro_mastro->cur_index+i].quantita,libro_mastro->trn[libro_mastro->cur_index+i].reward,libro_mastro->trn[libro_mastro->cur_index+i].timestamp);
                        //sleep(2);
                    }

                    libro_mastro->cur_index+=(SO_BLOCK_SIZE);
                    
                    if(libro_mastro->cur_index>=(SO_REGISTRY_SIZE*SO_BLOCK_SIZE))
                    {
                        kill(getppid(),SIGUSR1);
                        TEST_ERROR;
                    }

                    my_sem_lm.sem_num=0;
                    my_sem_lm.sem_op=1;
                    semop(IDSemLm,&my_sem_lm,1);
                    TEST_ERROR;

                    for(z=0;z<indice_ultimo_elemento_tp-(SO_BLOCK_SIZE-1);z++)
                    {
                        transaction_pool[z].quantita=transaction_pool[(SO_BLOCK_SIZE)+z].quantita;
                        transaction_pool[z].sender=transaction_pool[(SO_BLOCK_SIZE)+z].sender;
                        transaction_pool[z].receiver=transaction_pool[(SO_BLOCK_SIZE)+z].receiver;
                        transaction_pool[z].reward=transaction_pool[(SO_BLOCK_SIZE)+z].reward;
                        transaction_pool[z].timestamp=transaction_pool[(SO_BLOCK_SIZE)+z].timestamp;
                        //printf("elemento %d del transaction pool: %d",indice_ultimo_elemento_tp-z,transaction_pool[z].quantita);
                    }
                    indice_ultimo_elemento_tp-=(SO_BLOCK_SIZE);
                }
            }
            if(indice_ultimo_elemento_tp==SO_TP_SIZE-1)
            {
                kill(msg.trn.sender,SIGBUS);
            }
        }
    }
}


void sig_handler(int signum){
    fflush(stdout);
    printf("\nELEMENTI PRESENTI NELLA TRANSACTION POOL DI %d: %d\n",getpid(),indice_ultimo_elemento_tp+1);
    fflush(stdout);
    exit(EXIT_SUCCESS);
}