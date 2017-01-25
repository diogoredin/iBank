#ifndef TAREFAS_H
#define TAREFAS_H

/****************************************************************************************
*
*   1. Bibliotecas
*
*****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>

/****************************************************************************************
*
*   2. Constantes
*
*****************************************************************************************/

#define NI 0
#define OP_DEBITAR 1
#define OP_CREDITAR 2
#define OP_LER_SALDO 3
#define OP_TRANSFERIR 4
#define OP_SIMULAR 5
#define OP_SAIR 6
#define OP_SAIR_AGORA 7

#define COMANDO_DEBITAR "debitar"
#define COMANDO_CREDITAR "creditar"
#define COMANDO_LER_SALDO "lerSaldo"
#define COMANDO_TRANSFERIR "transferir"
#define COMANDO_SIMULAR "simular"
#define COMANDO_SAIR "sair"
#define COMANDO_AGORA "agora"

#define NUM_TRABALHADORAS 3
#define CMD_BUFFER_DIM (NUM_TRABALHADORAS * 2)

#define FALSE (1==0)
#define TRUE  (1==1)

/****************************************************************************************
*
*   3. Variaveis
*
*****************************************************************************************/

/* Estrutura do Comando */
typedef struct {
	int terminal;
	int id;
	time_t tempo;
	int operacao;
	int idConta_a;
	int idConta_b;
	int valor;
} comando_t;

/* Guarda IDs das Tarefas Existentes */
pthread_t tid[NUM_TRABALHADORAS];

/* Guarda comandos a executar */
comando_t cmd_buffer[CMD_BUFFER_DIM];

/* Indice de leitura e escrita no buffer */
int buff_write_idx, buff_read_idx;

/* Semaforos dos produtores e consumidores */
sem_t semPodeProd;
sem_t semPodeCons;

/* Mutex do Buffer e Condition Associada */
pthread_mutex_t buffer_mutex;
pthread_cond_t cond;

/* Ficheiro de Registos */
int registo;

/****************************************************************************************
*
*   4. Prototipos
*
*****************************************************************************************/

comando_t cria_comando( 
	int terminal, int id, time_t tempo, int operacao, int idConta_a, int idConta_b, int valor );

void threads_init();
void circular_buffer_init();
int circular_buffer_empty();
void circular_buffer_destroy();

void produtor( comando_t comando );
void *consumidor( void*(data) );

/* Definidos no i-banco */
void comando_debitar( int idConta, int valor );
void comando_creditar( int idConta, int valor );
void comando_sair( int tiposaida );
void comando_ler_saldo( int idConta );
void comando_transferir( int idConta_a, int idConta_b, int valor );
void comando_simular( int numAnos, comando_t comando );

#endif