#include "headers.h"

/**********************   Diogo Redin 84711 * Gonçalo Cruz 84721   ***********************
*
*
*	PROGRAMA I-BANCO - PROJETO SO DEI/IST/ULisboa 2016-17
*	http://goo.gl/2Dms5g
*
*
*****************************************************************************************/

/****************************************************************************************
*
*	INDEX
*
*	I-BANCO.C
*
*	1. CICLO PRINCIPAL
*	2. FUNCOES AUXILIARES
*		2.1. ABRE REGISTO
*		2.2. CRIA PIPE IBANCO
*		2.3. ABRE PIPE IBANCO
*		2.4. REDIRECIONA OUTPUT
*		2.5. FECHA PIPE
*		2.6. SIGNAL HANDLERS
*	3. COMANDOS
*		3.1. COMANDO DEBITAR
*		3.2. COMANDO CREDITAR
*		3.3. COMANDO LER SALDO
*		3.4. COMANDO TRANSFERIR
*		3.5. COMANDO SAIR
*		3.6. COMANDO SIMULAR
*
*	CONTAS.C
*
*	1. TRINCOS
*		1.1. INICIALIZA TRINCO
*		1.2. DESTROI TRINCOS CONTAS
*	2. OPERACOES CONTAS
*		2.1. INICIALIZA CONTAS
*		2.2. CONTA EXISTE
*		2.3. MENOR ID CONTA
*		2.4. MAIOR ID CONTA
*		2.3. DEBITAR
*		2.4. CREDITAR
*		2.5. LER SALDO
*		2.6. TRANSFERIR
*		2.7. SIMULAR
*
*	TAREFAS.C
*
*	1. CRIA COMANDO
*	2. THREADS INIT
*	3. CIRCULAR BUFFER
*		3.1. INICIALIZA BUFFER
*		3.2. BUFFER VAZIO
*		3.3. DESTROI BUFFER
*	4. PRODUTOR
*	5. CONSUMIDOR
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

	/* Variaveis das Pipes */
	comando_t comando;
	char ibanco_pipe[50];
	char terminal_pipe[50];
	int ibanco, terminal, retrn;

	/* Inicializa contas, tarefas e buffer de comandos */
	contas_init();
	circular_buffer_init();
	threads_init();

	/* Abre Registo */
	abre_registo();

	/* Cria e Abre Pipe iBanco (<-) */
	strcpy( ibanco_pipe, cria_ibanco() );
	ibanco = abre_ibanco( ibanco_pipe );

	while (1) {

		/* Associa funcao de termino do processo filho a sinal */
		if ( signal( SIGUSR1, terminar_processo ) == SIG_ERR) {
			perror("Erro ao definir signal.");
			exit(EXIT_FAILURE);
		}

		/* Le comando (<-) */
		read(ibanco, &comando, sizeof(comando));

		/* Redireciona Output para Pipe Terminal (->) */
		sprintf(terminal_pipe, "/tmp/ibanco_terminal_%d", comando.terminal);
		terminal = redireciona_output( terminal_pipe );

		/* COMANDO SAIR */
		if ( comando.operacao == OP_SAIR ) {

			comando_sair( 0 );

			/* Liberta Recursos */
			fecha_pipe(terminal);
			fecha_pipe(ibanco);
			unlink(terminal_pipe);
			unlink(ibanco_pipe);

			exit(EXIT_SUCCESS);
		}

		/* COMANDO SAIR AGORA */
		else if ( comando.operacao == OP_SAIR_AGORA ) {

			comando_sair( 1 );

			/* Liberta Recursos */
			fecha_pipe(terminal);
			fecha_pipe(ibanco);
			unlink(terminal_pipe);
			unlink(ibanco_pipe);

			exit(EXIT_SUCCESS);
		}

		/* COMANDO DEBITAR */
		else if ( comando.operacao == OP_DEBITAR ) {
			produtor( comando );
		}

		/* COMANDO CREDITAR */
		else if ( comando.operacao == OP_CREDITAR ) {
			produtor( comando );
		}

		/* COMANDO LER SALDO */
		else if ( comando.operacao == OP_LER_SALDO ) {
			produtor( comando );
		}

		/* COMANDO TRANSFERIR */
		else if ( comando.operacao == OP_TRANSFERIR ) {
			produtor( comando );
		}

		/* COMANDO SIMULAR */
		else if ( comando.operacao == OP_SIMULAR ) {

			/* Verifica se atingimos maximo processos */
			if (number_of_processes >= NUM_PROCESSOS) {
				printf("%s: Atingido o numero maximo de processos filho a criar.\n\n", COMANDO_SIMULAR);
				continue;
			}

			/* Bloqueia alteracoes / acesso ao buffer */
			retrn = pthread_mutex_lock(&buffer_mutex);
			if ( retrn != 0 ) {
				printf("%s : Erro a bloquear o trinco.\n", strerror(retrn));
			}

			/* Liberta o trinco e espera enquanto buffer nao esta vazio */
			while ( circular_buffer_empty() == FALSE ) {
				retrn = pthread_cond_wait(&cond, &buffer_mutex);
				if ( retrn != 0 ) {
					printf("%s : Erro no bloqueio da variavel de condicao.\n", strerror(retrn));
				}
			}

			/* Corre simulacao */
			comando_simular( comando.valor, comando );

			/* Desbloqueia alteracoes / acesso ao buffer */
			retrn = pthread_mutex_unlock(&buffer_mutex);
			if ( retrn != 0 ) {
				printf("%s : Erro a desbloquear o trinco.\n", strerror(retrn));
			}

		}

		/* Fecha Pipe Terminal */
		fecha_pipe(terminal);

	}

}

/****************************************************************************************
*
*   2.1. ABRE REGISTO
*	Cria / abre ficheiro para guardar comandos executados por tarefas.
*
*****************************************************************************************/

void abre_registo() {

	/* File Descriptor guardado em variavel global para acesso posterior */
	registo = open("log.txt", O_RDWR | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR );
	if ( registo == -1 ) {
		printf("%s : Erro a criar / ou abrir ficheiro log.txt.\n", strerror(registo));
		exit(EXIT_FAILURE);
	}

}

/****************************************************************************************
*
*   2.2. CRIA PIPE IBANCO
*	Cria pipe por onde ira receber comandos a executar.
*
*****************************************************************************************/

char* cria_ibanco() {

	int retrn;

	/* Tenta criar pipe no diretorio do programa */
	char* ibanco_pipe = "i-banco-pipe";
	unlink(ibanco_pipe);
	retrn = mkfifo(ibanco_pipe, S_IRUSR | S_IWUSR);
	if ( retrn == -1 ) {

		/* Temporario */
		if ( ( errno == EACCES ) || ( errno == EPERM ) ) {
			ibanco_pipe = "/tmp/i-banco-pipe";
			unlink(ibanco_pipe);
			retrn = mkfifo(ibanco_pipe, S_IRUSR | S_IWUSR );
			if ( retrn == -1 && ( ( errno == EACCES ) || ( errno == EPERM ) ) ) { 
				printf("\n%s : Erro a criar pipe para o iBanco no diretorio %s.\n\n", strerror(errno), ibanco_pipe);
				exit(EXIT_FAILURE);
			}

		} else {
			printf("\n%s : Erro a criar pipe para o iBanco no diretorio %s.\n\n", strerror(errno), ibanco_pipe);
			exit(EXIT_FAILURE);
		}

	}

	/* Sucesso */
	printf("\nSUCESSO : Pipe do iBanco criada com sucesso no directorio '%s'.\n\n", ibanco_pipe);
	return ibanco_pipe;

}

/****************************************************************************************
*
*   2.3. ABRE PIPE IBANCO
*	Abre pipe por onde ira receber comandos a executar.
*
*****************************************************************************************/

int abre_ibanco( char* ibanco_pipe ) {

	int fd = open(ibanco_pipe, O_RDWR, S_IRUSR | S_IWUSR );
	if ( fd == -1 ) {
		printf("%s : Erro a abrir pipe.\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return fd;

}

/****************************************************************************************
*
*   2.4. REDIRECIONA OUTPUT
*	Abre pipe e envia stdOut para o terminal.
*
*****************************************************************************************/

int redireciona_output( char* terminal_pipe ) {

	/* Abre terminal */
	int fd = open(terminal_pipe, O_RDWR, S_IRUSR | S_IWUSR);
	if ( fd == -1 ) {
		printf("%s : Erro a abrir a pipe para o iBanco terminal.\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* Redireciona Standard Output para Terminal */
	dup2(fd, STDOUT_FILENO);

	return fd;

}

/****************************************************************************************
*
*   2.5. FECHA PIPE
*	Fecha pipe especificada como argumento.
*
*****************************************************************************************/

void fecha_pipe( int fd ) {

	int retrn = close(fd);
	if ( retrn == -1 ) {
		printf("%s : Erro a fechar pipe.\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

}

/****************************************************************************************
*
*   2.6. SIGNAL HANDLERS
*	Trata sinal enviado quando queremos 'sair agora'.
*
*****************************************************************************************/

void terminar_processo(int signal) {

	if ( signal == SIGUSR1 ) {

		/* Definimos flag (acessivel dentro do msm processo) para sair */
		sair_agora = 1;

	}

}

/****************************************************************************************
*
*   3.1. COMANDO DEBITAR
*	Debita um determinado valor na conta especificada.
*
*****************************************************************************************/

void comando_debitar( int idConta, int valor ) {

	/* Debita Valor */
	if (debitar (idConta, valor) < 0) {
		printf("%s(%d, %d): Erro\n\n", COMANDO_DEBITAR, idConta, valor);
	} else {
		printf("%s(%d, %d): OK\n\n", COMANDO_DEBITAR, idConta, valor);
	}

}

/****************************************************************************************
*
*   3.2. COMANDO CREDITAR
*	Credita um determinado valor na conta especificada.
*
*****************************************************************************************/

void comando_creditar( int idConta, int valor ) {

	/* Credita Valor */
	if (creditar (idConta, valor) < 0) {
		printf("%s(%d, %d): Erro\n\n", COMANDO_CREDITAR, idConta, valor);
	} else {
		printf("%s(%d, %d): OK\n\n", COMANDO_CREDITAR, idConta, valor);
	}

}

/****************************************************************************************
*
*   3.3. COMANDO LER SALDO
*	Le o saldo de uma determinada conta especificada.
*
*****************************************************************************************/

void comando_ler_saldo( int idConta ) {

	int saldo = lerSaldo(idConta);

	/* Le Saldo da Conta */
	if (saldo < 0) {
		printf("%s(%d): Erro.\n\n", COMANDO_LER_SALDO, idConta);
	} else {
		printf("%s(%d): O saldo da conta é %d.\n\n", COMANDO_LER_SALDO, idConta, saldo);
	}

}

/****************************************************************************************
*
*   3.4. COMANDO TRANSFERIR
*	Transfere um dado valor de uma conta a para uma conta b (->).
*
*****************************************************************************************/

void comando_transferir( int idConta_a, int idConta_b, int valor ) {

	/* Transfere Valor */
	if (transferir(idConta_a, idConta_b, valor) < 0) {
		printf("Erro ao transferir %d da conta %d para a conta %d.\n\n", valor, idConta_a, idConta_b);
	} else {
		printf("transferir(%d, %d, %d): OK\n\n", idConta_a, idConta_b, valor);
	}

}

/****************************************************************************************
*
*   3.5. COMANDO SAIR
*	Espera que os processos do programa terminem para sair.
*
*****************************************************************************************/

void comando_sair( int tiposaida ) {

	/* Variaveis Temporarias processos */
	int i;
	int pid;
	int status;

	/* Estado saida tarefas */
	void* status_saida_tarefas;
	int retrn;

	/* Cria Comando que termina tarefa trabalhadora */
	comando_t comando_termina;

	printf("\ni-banco vai terminar.\n--\n");

	/*************************************************
	* TERMINA TAREFAS
	*************************************************/

	/* Cria Comando que termina tarefa trabalhadora */
	comando_termina = cria_comando(NI, NI, NI, OP_SAIR, NI, NI, NI);

	/* Envia Comando a todas as tarefas trabalhadoras */
	for (i=1; i <= NUM_TRABALHADORAS; i++) {
		produtor( comando_termina );
	}

	/* Verifica se todas foram efetivamente terminadas */
	for (i=1; i <= NUM_TRABALHADORAS; i++) {
		retrn = pthread_join( tid[i], &status_saida_tarefas );
		if ( retrn != 0 ) {
			printf("%s : Tarefa %lu não terminada corretamente.\n", strerror( *(int*) status_saida_tarefas ), tid[i]);
		}
	}

	/* Fecha ficheiro registos */
	retrn = close(registo);
	if ( retrn == -1 ) {
		printf("%s : Erro a fechar ficheiro log.txt.\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	/*************************************************
	* TERMINA PROCESSOS
	*************************************************/

	/* Sai Abruptamente Quando Terminar Ano Corrente */
	if (tiposaida == 1) {

		/* Percorremos Processos e Informamos que queremos Sair */
		for (i = 0; i < number_of_processes; i++) {

			pid = processes[i];
			status = kill(pid, SIGUSR1);

			/* Houve Erro */
			if ( status == -1 ) {
				printf("ERRO : Nao foi possivel sair de todos os processos.\n");
				return;
			}

		}

	}

	/* Percorremos Processos */
	for (i = 0; i < number_of_processes; i++) {

		pid = processes[i];

		/* Espera que o processo acabe normalmente (so se nao tiver saido abruptamente) */
		waitpid(pid, &status, 0);

		/* Houve Erro */
		if ( status == -1 ) {
			printf("ERRO : Nao foi possivel sair de todos os processos.\n");
			return;
		}

		/* Se for saida abrupta e nao tenha ja terminado normalmente */
		if ( (tiposaida == 1) && WIFEXITED(status) && ( WEXITSTATUS(status) != 0 ) ) {
			printf("Simulacao terminada por signal\n");
		}

		/* Se tiver terminado por Exit */
		if ( WIFEXITED(status) ) {
			printf("FILHO TERMINADO (PID=%d; terminado normalmente)\n", pid);
		} else {
			printf("FILHO TERMINADO (PID=%d; terminado abruptamente)\n", pid);
		}

	}

	printf("--\ni-banco terminou.\n\n");

	/* Como vamos sair libertamos recursos utilizados */
	trincos_contas_destroy();
	circular_buffer_destroy();

}

/****************************************************************************************
*
*   3.6. COMANDO SIMULAR
*	Simula a evolucao dos saldos das contas nos proximos numAnos especificados.
*
*****************************************************************************************/

void comando_simular( int numAnos, comando_t comando ) {

	pid_t pid;

	/* Variaveis das Pipes */
	char executados_pipe[50];
	int executados, comando_id, retrn;

	/* Cria Processo para Simulacao */
	pid = fork();

	/* Erro a Criar o Processo */
	if ( pid < 0 ) {
		printf("%d: Impossivel simular / criar novo processo.\n", pid);
	}

	/* Processo Filho */
	else if ( pid == 0 ) {

		/* Comando recebido */
		printf("simular(%d): OK\n\n", numAnos);

		/* Inicia Flag */
		sair_agora = 0;

		/* Simula */
		simular( numAnos );

		/* Abre Pipe Comandos Executados (->) */
		sprintf(executados_pipe, "/tmp/ibanco_terminal_executados_%d", comando.terminal);
		executados = open(executados_pipe, O_RDWR, S_IRUSR | S_IWUSR);
		if ( executados == -1 ) {
			printf("%s : Erro a abrir a pipe para o iBanco terminal.\n", strerror(errno));
			exit(EXIT_FAILURE);
		}

		/* Envia para a pipe o comando ID (->) */
		comando_id = comando.id;
		retrn = write(executados, &comando_id, sizeof(comando_id));
		if ( retrn == -1 ) {
			printf("%s : Erro a comunicar com o iBanco terminal.\n", strerror(errno));
			exit(EXIT_FAILURE);
		}

		/* Informa terminal do termino da execucao do comando */
		retrn = kill(comando.terminal, SIGUSR1);
		if ( retrn == -1 ) {
			printf("ERRO : Falha a informar fim de execucao do comando.\n");
		}

		/* Fecha iBanco Pipe Comandos Executados */
		retrn = close(executados);
		if ( retrn == -1 ) {
			printf("%s : Erro a fechar o pipe para o iBanco.\n", strerror(errno));
			exit(EXIT_FAILURE);
		}

		/* Termina Processo */
		exit(EXIT_SUCCESS);

	}

	/* Processo Pai */
	else if ( pid != 0 ) {

		/* Guarda Novo Processo Filho no Vetor */
		processes[number_of_processes] = pid;

		/* Aumenta Numero Processos */
		number_of_processes++;

	}

}