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
#include <sys/wait.h>
#include <sys/stat.h>

/****************************************************************************************
*
*   2. Dependencias
*
*****************************************************************************************/

#include "commandlinereader.h"
#include "contas.h"
#include "tarefas.h"

/****************************************************************************************
*
*   3. Constantes
*
*****************************************************************************************/

#define MAXARGS 4
#define BUFFER_SIZE 100

#define NUM_PROCESSOS 10

#define FALSE (1==0)
#define TRUE  (1==1)

/****************************************************************************************
*
*   4. Variaveis
*
*****************************************************************************************/

int processes[NUM_PROCESSOS];
int number_of_processes = 0;

/****************************************************************************************
*
*   5. Prototipos
*
*****************************************************************************************/

void abre_registo();
char* cria_ibanco();
int abre_ibanco( char* ibanco_pipe );
int redireciona_output( char* terminal_pipe );
void fecha_pipe( int fd );
void terminar_processo( int signal );

#endif