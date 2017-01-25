#ifndef CONTAS_H
#define CONTAS_H

/****************************************************************************************
*
*   1. Bibliotecas
*
*****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>

/****************************************************************************************
*
*   2. Constantes
*
*****************************************************************************************/

#define atrasar() sleep(ATRASO)

#define NUM_CONTAS 10
#define TAXAJURO 0.1
#define CUSTOMANUTENCAO 1

#define ATRASO 1

/****************************************************************************************
*
*   3. Variaveis
*
*****************************************************************************************/

int contasSaldos[NUM_CONTAS];

/* Esta Flag pertence aos processos filhos e e' ativada sempre que e enviado um signal
a um processo filho. 0 significa que nao queremos 'sair agora' e 1 que pretendemos sair
o mais rapidamente possivel (assim q terminamos o ano em que estamos). */
int sair_agora;

/* Estrutura de Trinco de uma Conta */
typedef struct {
	pthread_mutex_t mutex;
	int inicializado;
} trinco_conta;

/* Guarda Trincos das Contas */
trinco_conta trincos_contas[NUM_CONTAS];

/****************************************************************************************
*
*   4. Prototipos
*
*****************************************************************************************/

void trinco_conta_init( int idConta );
void trincos_contas_destroy();

void contas_init();
int contaExiste(int idConta);
int menor(int idConta_a, int idConta_b);
int maior(int idConta_a, int idConta_b);

int debitar(int idConta, int valor);
int creditar(int idConta, int valor);
int lerSaldo(int idConta);
int transferir(int idConta_a, int idConta_b, int valor);
void simular(int numAnos);

#endif