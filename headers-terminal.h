#ifndef HEADERS_H
#define HEADERS_H

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
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

/****************************************************************************************
*
*   2. Dependencias
*
*****************************************************************************************/

#include "commandlinereader.h"

/****************************************************************************************
*
*   3. Constantes
*
*****************************************************************************************/

#define MAXARGS 4
#define BUFFER_SIZE 100

#define FALSE (1==0)
#define TRUE  (1==1)

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
#define COMANDO_SAIR_AGORA "agora"
#define COMANDO_SAIR_TERMINAL "sair-terminal"

/****************************************************************************************
*
*   4. Variaveis
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

/* Pipe que Recebe stdOut */
char terminal_pipe[50];
int terminal;

/* Pipe que Recebe Comandos Executados */
char executados_pipe[50];
int executados;

/* Guarda Comandos Enviados */
comando_t* comandos_enviados;
int nr_comandos_enviados;

/****************************************************************************************
*
*   5. Prototipos
*
*****************************************************************************************/

comando_t cria_comando( 
	int terminal, int id, time_t tempo, int operacao, int idConta_a, int idConta_b, int valor );

int abre_ibanco( int argc, char* pipe );
void cria_terminal_executados( int pid );
int abre_terminal_executados( char* ibanco_executados_pipe );
void fecha_pipe( int fd );

int imprime_resultados( int processo );
int verifica_input( char* char_input );

void envia_comando( int fd, comando_t comando, size_t tamanho );
void guarda_comando_enviado( comando_t comando, int tamanho );

void comando_terminou( int signal );
void commands_output( int processo );
void commands_output_exit( int signal );

#endif