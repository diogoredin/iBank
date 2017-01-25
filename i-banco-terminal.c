#include "headers-terminal.h"

/**********************   Diogo Redin 84711 * Gonçalo Cruz 84721   ***********************
*
*
*	PROGRAMA I-BANCO-TERMINAL - PROJETO SO DEI/IST/ULisboa 2016-17
*	http://goo.gl/2Dms5g
*
*
*****************************************************************************************/

/****************************************************************************************
*
*	INDEX
*
*	I-BANCO-TERMINAL.C
*
*	1. CICLO PRINCIPAL
*	2. FUNCOES AUXILIARES
*		2.1. ABRE PIPE IBANCO
*		2.2. CRIA PIPE IBANCO EXECUTADOS
*		2.3. ABRE PIPE IBANCO EXECUTADOS
*		2.4. IMPRIME RESULTADOS IBANCO
*		2.5. VERIFICA INPUT
*		2.6. CRIA COMANDO
*		2.7. ENVIA COMANDO
*		2.8. GUARDA COMANDO
*		2.9. FECHA PIPE
*		2.10. COMANDO TERMINOU
*		2.11. COMMANDS OUTPUT
*		2.13. COMMANDS OUTPUT EXIT
*
*	COMMANDLINEREADER.C
*
*	1. COMMAND LINE READER
*
****************************************************************************************/

/****************************************************************************************
*
*   1. CICLO PRINCIPAL
*
*****************************************************************************************/

int main(int argc, char** argv) {

	/* Variaveis do Programa */
	time_t start_t;
	pid_t pid;

	/* Variaveis dos Command Line Reader */
	int numargs;
	char *args[MAXARGS + 1];
	char buffer[BUFFER_SIZE];

	/* Variaveis das Pipes */
	comando_t comando;
	int ibanco, retrn;

	/* Guarda Comandos Enviados */
	comandos_enviados = malloc(1 * sizeof(comando_t));

	/* Abre iBanco (->) */
	ibanco = abre_ibanco( argc, argv[1] );

	/* Imprime Continamente stdOut vindo do iBanco p/ este terminal */
	pid = imprime_resultados( getpid() );

	/* Cria e Abre iBanco Terminal Executados Pipe (<-) */
	cria_terminal_executados( getpid() );
	executados = abre_terminal_executados( executados_pipe );

	/*************************************************
	* IBANCO TERMINAL
	*************************************************/

	printf("\nBem-vinda/o ao terminal do iBanco\n\n");
	while (1) {

		/* Le comando do terminal */
		numargs = readLineArguments(args, MAXARGS+1, buffer, BUFFER_SIZE);

		/* Regista tempo de inicio da execucao */
		time(&start_t);

		/* Associa funcao de termino do comando a sinal */
		if ( signal( SIGUSR1, comando_terminou ) == SIG_ERR) {
			perror("Erro ao definir signal.");
			exit(EXIT_FAILURE);
		}

		/* COMANDO SAIR OU EOF */
		if ( numargs < 0 || ( (numargs == 1) && (strcmp(args[0], COMANDO_SAIR) == 0)) ) {

			/* Cria e Envia Comando */
			comando = cria_comando( getpid(), NI, NI, OP_SAIR, NI, NI, NI );
			envia_comando(ibanco, comando, sizeof(comando));

		}

		/* COMANDO SAIR AGORA */
		else if ( (numargs == 2) && (strcmp(args[0], COMANDO_SAIR) == 0) 
			&& (strcmp(args[1], COMANDO_SAIR_AGORA) == 0) ) {

			/* Cria e Envia Comando */
			comando = cria_comando( getpid(), NI, NI, OP_SAIR_AGORA, NI, NI, NI );
			envia_comando(ibanco, comando, sizeof(comando));

		}

		/* COMANDO SAIR TERMINAL */
		else if ((numargs == 1 ) && (strcmp(args[0], COMANDO_SAIR_TERMINAL) == 0)) {

			/* Fecha Pipes Abertas */
			fecha_pipe(ibanco);
			fecha_pipe(executados);

			/* Informa processo que imprime output dos programas para sair */
			retrn = kill(pid, SIGUSR2);
			if ( retrn == -1 ) {
				printf("ERRO : Falha a informar fim de execucao do terminal.\n");
				exit(EXIT_FAILURE);
			}

			/* Liberta recursos e termina */
			unlink(executados_pipe);
			free(comandos_enviados);
			exit(EXIT_SUCCESS);

		}

		/* COMANDO NAO DADO */
		else if (numargs == 0) {
			continue;
		}

		/* COMANDO DEBITAR */
		else if (strcmp(args[0], COMANDO_DEBITAR) == 0) {

			/* Verifica Input */
			if ( numargs != 3 || !verifica_input(args[1]) || !verifica_input(args[2]) ) {
				printf("%s : Sintaxe inválida, tente de novo.\n\n", COMANDO_DEBITAR);
				continue;
			}

			/* Cria, Guarda e Envia Comando */
			comando = cria_comando( getpid(), ++nr_comandos_enviados, start_t, OP_DEBITAR, atoi(args[1]), NI, atoi(args[2]) );
			guarda_comando_enviado(comando, nr_comandos_enviados);
			envia_comando(ibanco, comando, sizeof(comando));

		}

		/* COMANDO CREDITAR */
		else if (strcmp(args[0], COMANDO_CREDITAR) == 0) {

			/* Verifica Input */
			if ( numargs != 3 || !verifica_input(args[1]) || !verifica_input(args[2]) ) {
				printf("%s : Sintaxe inválida, tente de novo.\n\n", COMANDO_CREDITAR);
				continue;
			}

			/* Cria, Guarda e Envia Comando */
			comando = cria_comando( getpid(), ++nr_comandos_enviados, start_t, OP_CREDITAR, atoi(args[1]), NI, atoi(args[2]) );
			guarda_comando_enviado(comando, nr_comandos_enviados);
			envia_comando(ibanco, comando, sizeof(comando));

		}

		/* COMANDO LER SALDO */
		else if (strcmp(args[0], COMANDO_LER_SALDO) == 0) {

			/* Verifica Input */
			if ( numargs != 2 || !verifica_input(args[1]) ) {
				printf("%s : Sintaxe inválida, tente de novo.\n\n", COMANDO_LER_SALDO);
				continue;
			}

			/* Cria, Guarda e Envia Comando */
			comando = cria_comando( getpid(), ++nr_comandos_enviados, start_t, OP_LER_SALDO, atoi(args[1]), NI, NI );
			guarda_comando_enviado(comando, nr_comandos_enviados);
			envia_comando(ibanco, comando, sizeof(comando));

		}

		/* COMANDO TRANSFERIR */
		else if (strcmp(args[0], COMANDO_TRANSFERIR) == 0) {

			/* Verifica Input */
			if ( numargs != 4 || !verifica_input(args[1]) || !verifica_input(args[2]) || !verifica_input(args[3]) ) {
				printf("%s : Sintaxe inválida, tente de novo.\n\n", COMANDO_TRANSFERIR);
				continue;
			}

			/* Cria, Guarda e Envia Comando */
			comando = cria_comando( getpid(), ++nr_comandos_enviados, start_t, OP_TRANSFERIR, atoi(args[1]), atoi(args[2]), atoi(args[3]) );
			guarda_comando_enviado(comando, nr_comandos_enviados);
			envia_comando(ibanco, comando, sizeof(comando));

		}

		/* COMANDO SIMULAR */
		else if (strcmp(args[0], COMANDO_SIMULAR) == 0) {

			/* Verifica Input */
			if ( numargs != 2 || !verifica_input(args[1]) ) {
				printf("%s: Sintaxe inválida, tente de novo.\n\n", COMANDO_SIMULAR);
				continue;
			}

			/* Cria, Guarda e Envia Comando */
			comando = cria_comando( getpid(), ++nr_comandos_enviados, start_t, OP_SIMULAR, NI, NI, atoi(args[1]) );
			guarda_comando_enviado(comando, nr_comandos_enviados);
			envia_comando(ibanco, comando, sizeof(comando));

		}

		/* COMANDO DESCONHECIDO */
		else {
			printf("Comando desconhecido. Tente de novo.\n\n");
		}

	}

}

/****************************************************************************************
*
*   2.1. ABRE PIPE IBANCO
*	Abre iBanco Pipe por onde obtem comandos a executar.
*
*****************************************************************************************/

int abre_ibanco( int argc, char* pipe ) {

	int fd;

	if ( argc == 2 ) {
		fd = open(pipe, O_RDWR, S_IRUSR | S_IWUSR);
		if ( fd == -1 ) {
			printf("%s : Erro a abrir o pipe para o iBanco.\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
	} else {
		printf("Erro : Localizacao da pipe nao especificada.\n");
		exit(EXIT_FAILURE);
	}

	return fd;
}

/****************************************************************************************
*
*   2.2. CRIA PIPE IBANCO EXECUTADOS
*	Cria Pipe do iBanco para o Terminal por onde este recebe comandos executados.
*
*****************************************************************************************/

void cria_terminal_executados( int pid ) {

	int retrn;

	sprintf(executados_pipe, "/tmp/ibanco_terminal_executados_%d", pid);
	unlink(executados_pipe);

	retrn = mkfifo(executados_pipe, S_IRUSR | S_IWUSR );
	if ( retrn == -1 ) {
		printf("\n%s : Erro a criar pipe para o iBanco no diretorio %s.\n\n", strerror(errno), executados_pipe);
		exit(EXIT_FAILURE);
	}

}

/****************************************************************************************
*
*   2.3. ABRE PIPE IBANCO EXECUTADOS
*	Abre Pipe do iBanco para o Terminal por onde este recebe comandos executados.
*
*****************************************************************************************/

int abre_terminal_executados( char* ibanco_executados_pipe ) {

	int fd = open(ibanco_executados_pipe, O_RDWR, S_IRUSR | S_IWUSR);
	if ( fd == -1 ) {
		printf("%s : Erro a abrir a pipe para o iBanco terminal.\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return fd;
}

/****************************************************************************************
*
*   2.4. IMPRIME RESULTADOS IBANCO
*	Cria processo paralelo que imprime continuamente stdOut do iBanco p/ este terminal.
*
*****************************************************************************************/

int imprime_resultados( int processo ) {

	/* Cria Processo */
	int pid = fork();

	/* Processo Filho */
	if ( pid == 0 ) {

		/* Imprime Continuamente da Pipe que vem do iBanco */
		commands_output( processo );

		/* Associa sinal a funcao de termino deste processo */
		if ( signal( SIGUSR2, commands_output_exit ) == SIG_ERR) {
			perror("Erro ao definir signal.");
			exit(EXIT_FAILURE);
		}

	} else if ( pid < 0 ) { 
		printf("%d : Impossivel criar novo processo.\n", pid); 
	}

	return pid;
}

/****************************************************************************************
*
*   2.5. VERIFICA INPUT
*	Retorna positivo quando nao ha erros no input introduzido.
*
*****************************************************************************************/

int verifica_input( char* char_input ) {

	int nr_digitos = 0;
	int numero = atoi(char_input);
	char caracter = char_input[0];

	/* Conta Digitos */
	if (numero != 0) {
		while (numero != 0) {
			numero /= 10;
			nr_digitos++;
		}
	} else { nr_digitos = 1; }

	/* 1 - Nao pode retornar numero negativo */
	/* 2 - Se o numero introduzido nao for 0 mas a funcao atoi retornar 0 houve erro */
	/* 3 - Se o nr de digitos diferir do tamanho do argumento houve caracteres alfabeticos 
	escritos antes ou depois do nr */
	if ( atoi( char_input ) < 0 ||
		( caracter != '0' && atoi(char_input) == 0 ) || 
		nr_digitos != strlen( char_input ) ) {
		return FALSE;
	} else {
		return TRUE;
	}

}

/****************************************************************************************
*
*   2.6. CRIA COMANDO
*	Dadas as informacoes necessarias cria um comando.
*
*****************************************************************************************/

comando_t cria_comando(
	int terminal, int id, time_t tempo, int operacao, int idConta_a, int idConta_b, int valor ) {

	/* Cria Comando */
	comando_t comando;
	comando.terminal = terminal;
	comando.id = id;
	comando.tempo = tempo;
	comando.operacao = operacao;
	comando.idConta_a = idConta_a;
	comando.idConta_b = idConta_b;
	comando.valor = valor;

	return comando;
}

/****************************************************************************************
*
*   2.7. ENVIA COMANDO
*	Envia comando para o file descriptor dado.
*
*****************************************************************************************/

void envia_comando( int fd, comando_t comando, size_t tamanho ) {

	int retrn;

	retrn = write(fd, &comando, tamanho);
	if ( retrn == -1 ) {
		printf("%s : Erro a comunicar com o iBanco.\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

}

/****************************************************************************************
*
*   2.8. GUARDA COMANDO
*	Reserva espaco e guarda comando enviado.
*
*****************************************************************************************/

void guarda_comando_enviado( comando_t comando, int tamanho ) {

	/* Reserva espaco para guardar comando enviado */
	comando_t* comandos_enviados_update;
	comandos_enviados_update = realloc(comandos_enviados, (tamanho+1) * sizeof(comando_t));

	/* Se for bem sucedido guarda comando */
	if ( comandos_enviados_update ) {
		comandos_enviados = comandos_enviados_update;
		comandos_enviados[tamanho] = comando;
	} else {
		printf("Erro a alocar memória para guardar comandos enviados.\n");
		exit(EXIT_FAILURE);
	}

}

/****************************************************************************************
*
*   2.9. FECHA PIPE
*	Fecha pipe especificada como argumento.
*
*****************************************************************************************/

void fecha_pipe( int fd ) {

	int retrn;

	retrn = close(fd);
	if ( retrn == -1 ) {
		printf("%s : Erro a fechar a pipe.\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

}

/****************************************************************************************
*
*   2.10. COMANDO TERMINOU
*	Mostra output e calcula tempo de execucao do comando quando recebe sinal.
*
*****************************************************************************************/

void comando_terminou( int signal ) {

	/* Tempo Final */
	time_t end_t;

	/* Variaveis da Pipe */
	comando_t comando;
	int comando_id;

	/* Recebeu sinal */
	if ( signal == SIGUSR1 ) {

		/* Le comandos terminados da pipe */
		read(executados, &comando_id, sizeof(comando_id));

		/* Obtem dados guardados do comando terminado */
		comando = comandos_enviados[comando_id];

		/* Calcula Tempo Terminado */
		time(&end_t);

		/* Calcula Tempo Execucao do Comando */
		if ( comando.operacao == OP_DEBITAR ) {
			printf("COMANDO TERMINADO (%lf s): %s(%d, %d) \n\n",
				(double) difftime( end_t, comando.tempo ), COMANDO_DEBITAR, comando.idConta_a, comando.valor);

		} else if ( comando.operacao == OP_CREDITAR) {
			printf("COMANDO TERMINADO (%lf s): %s(%d, %d) \n\n",
				(double) difftime( end_t, comando.tempo ), COMANDO_CREDITAR, comando.idConta_a, comando.valor );

		} else if ( comando.operacao == OP_LER_SALDO ) {
			printf("COMANDO TERMINADO (%lf s): %s(%d) \n\n",
				(double) difftime( end_t, comando.tempo ), COMANDO_LER_SALDO, comando.idConta_a );

		} else if ( comando.operacao == OP_TRANSFERIR ) {
			printf("COMANDO TERMINADO (%lf s): %s(%d, %d, %d) \n\n",
				(double) difftime( end_t, comando.tempo ), COMANDO_TRANSFERIR, comando.idConta_a, comando.idConta_b, comando.valor );

		} else if ( comando.operacao == OP_SIMULAR ) {
			printf("COMANDO TERMINADO (%lf s): %s(%d) \n\n",
				(double) difftime( end_t, comando.tempo ), COMANDO_SIMULAR, comando.valor );
		}

	}

}

/****************************************************************************************
*
*   2.11. COMMANDS OUTPUT
*	Imprime para o stdOut do terminal os resultados dados pelo iBanco atraves da pipe.
*
*****************************************************************************************/

void commands_output( int processo ) {

	/* Variaveis das Pipes */
	char buffer_output[50];
	int retrn;

	/* Associa funcao de termino do comando a sinal */
	if ( signal( SIGUSR2, commands_output_exit ) == SIG_ERR) {
		perror("Erro ao definir signal.");
		exit(EXIT_FAILURE);
	}

	/* Cria iBanco Terminal Pipe p/ stdOut (<-) */
	sprintf(terminal_pipe, "/tmp/ibanco_terminal_%d", processo);
	unlink(terminal_pipe);
	retrn = mkfifo(terminal_pipe, S_IRUSR | S_IWUSR );
	if ( retrn == -1 ) {
		printf("\n%s : Erro a criar pipe para o iBanco no diretorio %s.\n\n", strerror(errno), terminal_pipe);
		exit(EXIT_FAILURE);
	}

	/* Abre iBanco Terminal Pipe p/ stdOut (<-) */
	terminal = open(terminal_pipe, O_RDWR, S_IRUSR | S_IWUSR);
	if ( terminal == -1 ) {
		printf("%s : Erro a abrir o pipe para o iBanco terminal.\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* Le continuamente do iBanco */
	while (1) {
		read(terminal, buffer_output, sizeof(char));
		write(STDOUT_FILENO, &buffer_output, sizeof(char));
	}

}

/****************************************************************************************
*
*   2.13. COMMANDS OUTPUT EXIT
*	Quando recebe sinal fecha pipe com o iBanco (stdOut) e termina processo.
*
*****************************************************************************************/

void commands_output_exit( int signal ) {

	int retrn;

	/* Recebeu sinal */
	if ( signal == SIGUSR2 ) {

		/* Fecha iBanco Terminal Pipe (stdOut) - Cliente */
		retrn = close(terminal);
		if ( retrn == -1 ) {
			printf("%s : Erro a fechar o pipe para o iBanco terminal.\n", strerror(errno));
			exit(EXIT_FAILURE);
		}

		/* Liberta Recursos */
		unlink(terminal_pipe);

		/* Termina Processo */
		exit(EXIT_SUCCESS);

	}

}