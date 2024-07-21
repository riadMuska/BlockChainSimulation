#ifndef _CONF_H_
#include"conf.h"
#endif

#ifndef _FUNZ_H_
#define _FUNZ_H_
#include <stdbool.h>
void creazione_utente(int);
void creazione_nodo(int);
transizione new_transizione(long int, int,  int, int, int);
void set_timestamp(transizione ,long int);
int creaLibroMastro(key_t ,size_t );
int randomgen(int, int );
bool isFull(transizione*);
void insertFirst(transizione trn,struct node*);
void handle_signal(int);
void handler(int);
void sig_handler(int);
void sigTermHandler(int ,int );
void catch_sigterm(int);
#endif