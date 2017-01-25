#include "tarefas.h"

/****************************************************************************************
*
*	INDEX - tarefas.c
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
****************************************************************************************/

/****************************************************************************************
*
*   1. CRIA COMANDO
*	Dada uma operacao, idConta e valor cria um comando.
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
*   2. THREADS INIT
*	Inicializa um numero de tarefas predefinido e coloca-as num vetor.
*
*****************************************************************************************/

void threads_init() {

	/* Indice e Valor de Retorno */
	int i, retrn;

	/* Para cada trabalhadora predifinida criamos tarefa */
	for ( i=1; i <= NUM_TRABALHADORAS; i++ ) {

		/* Guarda tarefa criada com sucesso no vetor tid */
		retrn = pthread_create(&(tid[i]), NULL, consumidor, NULL);
		if ( retrn != 0 ) {
			printf("%s : Erro a criar tarefas predefinidas.\n", strerror(retrn));
			exit(EXIT_FAILURE);
		}

	}

}

/****************************************************************************************
*
*   3. CIRCULAR BUFFER
*	Inicializa/Destroi componentes do Buffer: Indices, Semaforos e Mutex.
*
*****************************************************************************************/

/* 3.1. Inicializa Buffer */
void circular_buffer_init() {

	/* Guarda Valor de Retorno */
	int retrn;

	/* Inicializamos Indices do Buffer a 0 */
	buff_write_idx = 0, buff_read_idx = 0;

	/* Criamos semaforo privado para produtores */
	retrn = sem_init(&semPodeProd, 0, CMD_BUFFER_DIM);
	if ( retrn == -1 ) {
		printf("%s : Erro a criar semaforo p/ produtores.\n", strerror(retrn));
		exit(EXIT_FAILURE);
	}

	/* Criamos semaforo privado para consumidores */
	retrn = sem_init(&semPodeCons, 0, 0);
	if ( retrn == -1 ) {
		printf("%s : Erro a criar semaforo p/ consumidores.\n", strerror(retrn));
		exit(EXIT_FAILURE);
	}

	/* Criamos Mutex para bloquear alteracoes ao buffer */
	retrn = pthread_mutex_init(&buffer_mutex, NULL);
	if ( retrn != 0 ) {
		printf("%s : Erro a criar mutex.\n", strerror(retrn));
		exit(EXIT_FAILURE);
	}

	/* Inicializamos variavel de condicao que sera associada ao Mutex do buffer */
	retrn = pthread_cond_init(&cond, NULL);
	if ( retrn != 0 ) {
		printf("%s : Erro a inicializar variavel de condicao.\n", strerror(retrn));
		exit(EXIT_FAILURE);
	}

}

/* 3.2. Buffer Vazio */
int circular_buffer_empty() {

	/* Retorna verdadeiro se houver algum comando */
	if ( buff_write_idx == buff_read_idx ){
		return TRUE;
	}

	return FALSE;
}

/* 3.3. Destroi Buffer */
void circular_buffer_destroy() {
	sem_destroy(&semPodeProd);
	sem_destroy(&semPodeCons);
	pthread_cond_destroy(&cond);
	pthread_mutex_destroy(&buffer_mutex);
}

/****************************************************************************************
*
*   4. PRODUTOR
*	- Corre na tarefa principal.
*	Coloca comando passado pelo STDin no buffer de comandos. 
*
*****************************************************************************************/

void produtor( comando_t comando ) {

	/* Guarda Valor de Retorno */
	int retrn;

	/* Espera que semaforo permita avancar na operacao */
	retrn = sem_wait(&semPodeProd);
	if ( retrn == -1 ) {
		printf("%s : Semaforo dos produtores tem valor inesperado.\n", strerror(retrn));
		exit(EXIT_FAILURE);
	}

	/* Bloqueia alteracoes / acesso ao buffer */
	retrn = pthread_mutex_lock(&buffer_mutex);
	if ( retrn != 0 ) {
		printf("%s : Erro a bloquear o trinco.\n", strerror(retrn));
		exit(EXIT_FAILURE);
	}

	/* Coloca comando no buffer circular - o indice e' o resto da divisao pelo maximo para 
	voltar ao inicio quando atingimos a ultima posicao */
	cmd_buffer[buff_write_idx] = comando;
	buff_write_idx = (buff_write_idx+1) % CMD_BUFFER_DIM;

	/* Desbloqueia alteracoes / acesso ao buffer */
	retrn = pthread_mutex_unlock(&buffer_mutex);
	if ( retrn != 0 ) {
		printf("%s : Erro a desbloquear o trinco.\n", strerror(retrn));
		exit(EXIT_FAILURE);
	}

	/* Altera estado semaforo dos consumidores para poderem prosseguir */
	retrn = sem_post(&semPodeCons);
	if ( retrn == -1 ) {
		printf("%s : Erro a alterar semaforo dos consumidores.\n", strerror(retrn));
		exit(EXIT_FAILURE);
	}

}

/****************************************************************************************
*
*   5. CONSUMIDOR
*	- Corre numa dada tarefa trabalhadora.
*	Processa um dado comando lido do buffer no indice indicado.
*
*****************************************************************************************/

void *consumidor(void*(data)) {
	while(TRUE) {

		/* Comando a executar */
		comando_t comando;
		int operacao, idConta_a, idConta_b, valor;

		/* Guarda valor de retorno */
		int retrn;

		/* Buffer e ficheiro de registo */
		char log_line[50];

		/* Variaveis da pipe */
		char ibanco_executados_pipe[50];
		int ibanco_executados;
		int comando_id;

		/* Espera que semaforo permita avancar na operacao */
		retrn = sem_wait(&semPodeCons);
		if ( retrn == -1 ) {
			printf("%s : Semaforo dos produtores tem valor inesperado.\n", strerror(retrn));
			exit(EXIT_FAILURE);
		}

		/* Bloqueia alteracoes / acesso ao buffer */
		retrn = pthread_mutex_lock(&buffer_mutex);
		if ( retrn != 0 ) {
			printf("%s : Erro a bloquear o trinco.\n", strerror(retrn));
			exit(EXIT_FAILURE);
		}

		/* Retira comando do buffer circular - o indice e' o resto da divisao pelo maximo para 
		voltar ao inicio quando atingimos a ultima posicao */
		comando = cmd_buffer[buff_read_idx];
		buff_read_idx = (buff_read_idx+1) % CMD_BUFFER_DIM;

		/* Informa que podemos prosseguir com tarefas pendentes */
		retrn = pthread_cond_signal(&cond);
		if ( retrn != 0 ) {
			printf("%s : Erro no signal enviado ao trinco do buffer.\n", strerror(retrn));
			exit(EXIT_FAILURE);
		}

		/* Desbloqueia alteracoes / acesso ao buffer */
		retrn = pthread_mutex_unlock(&buffer_mutex);
		if ( retrn != 0 ) {
			printf("%s : Erro a desbloquear o trinco.\n", strerror(retrn));
			exit(EXIT_FAILURE);
		}

		/* Altera estado semaforo dos consumidores para poderem prosseguir */
		retrn = sem_post(&semPodeProd);
		if ( retrn == -1 ) {
			printf("%s : Erro a alterar semaforo dos consumidores.\n", strerror(retrn));
			exit(EXIT_FAILURE);
		}

		/* Dados do Comando */
		operacao = comando.operacao;
		idConta_a = comando.idConta_a;
		idConta_b = comando.idConta_b;
		valor = comando.valor;

		/* Executa comanda e guarda comando executado */
		if ( operacao == OP_DEBITAR ) {
			comando_debitar( idConta_a, valor );
			sprintf(log_line, "%lu : %s(%d, %d)\n", pthread_self(), COMANDO_DEBITAR, idConta_a, valor);

		} else if ( operacao == OP_CREDITAR) {
			comando_creditar( idConta_a, valor );
			sprintf(log_line, "%lu : %s(%d, %d)\n", pthread_self(), COMANDO_CREDITAR, idConta_a, valor);

		} else if ( operacao == OP_LER_SALDO ) {
			comando_ler_saldo( idConta_a );
			sprintf(log_line, "%lu : %s(%d)\n", pthread_self(), COMANDO_LER_SALDO, idConta_a);

		} else if ( operacao == OP_TRANSFERIR ) {
			comando_transferir( idConta_a, idConta_b, valor );
			sprintf(log_line, "%lu : %s(%d, %d, %d)\n", pthread_self(), COMANDO_TRANSFERIR, idConta_a, idConta_b, valor);

		} else if ( operacao == OP_SAIR ) {
			pthread_exit( NULL );

		} else {
			printf("Erro : Comando não reconhecido.\n");
		}

		/* Escreve no ficheiro */
		retrn = write(registo, log_line, strlen(log_line));
		if ( retrn == -1 ) {
			printf("%s : Erro a escrever no ficheiro log.txt.\n", strerror(errno));
			exit(EXIT_FAILURE);
		}

		/* Abre iBanco Pipe (Comandos Executados) */
		sprintf(ibanco_executados_pipe, "/tmp/ibanco_terminal_executados_%d", comando.terminal);
		ibanco_executados = open(ibanco_executados_pipe, O_RDWR, S_IRUSR | S_IWUSR);
		if ( ibanco_executados == -1 ) {
			printf("%s : Erro a abrir a pipe para o iBanco terminal.\n", strerror(errno));
			exit(EXIT_FAILURE);
		}

		/* Envia para a pipe o comando ID */
		comando_id = comando.id;
		retrn = write(ibanco_executados, &comando_id, sizeof(comando_id));
		if ( retrn == -1 ) {
			printf("%s : Erro a comunicar com o iBanco terminal.\n", strerror(errno));
			exit(EXIT_FAILURE);
		}

		/* Informa terminal do termino da execucao do comando */
		retrn = kill(comando.terminal, SIGUSR1);
		if ( retrn == -1 ) {
			printf("ERRO : Falha a informar fim de execucao do comando.\n");
		}

		/* Fecha iBanco Pipe (Comandos Executados) */
		retrn = close(ibanco_executados);
		if ( retrn == -1 ) {
			printf("%s : Erro a fechar o pipe para o iBanco.\n", strerror(errno));
			exit(EXIT_FAILURE);
		}

	}
}