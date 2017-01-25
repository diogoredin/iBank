#include "contas.h"

/****************************************************************************************
*
*	INDEX - contas.c
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
****************************************************************************************/

/****************************************************************************************
*
*	1.1. INICIALIZA TRINCO - Inicializa trinco para uma determinada conta (inicializa
*	mutex e guarda na estrutura de trincos no indice correspondente ao indice da conta).
*
*****************************************************************************************/

void trinco_conta_init( int idConta ) {

	/* Guarda Valor de Retorno */
	int retrn;

	/* Se ainda nao foi Inicializado */
	if ( trincos_contas[idConta].inicializado != 1 ) {

		/* Inicializa Mutex que Guarda no Indice = nr Conta na estrutura trincos_contas */
		retrn = pthread_mutex_init(&trincos_contas[idConta].mutex, NULL);
		if ( retrn != 0 ) {
			printf("%s : Erro a criar mutex.\n", strerror(errno));
			exit(EXIT_FAILURE);
		} else {
			trincos_contas[idConta].inicializado = 1;
		}

	}

}

/****************************************************************************************
*
*	1.2. DESTROI TRINCOS - Percorre estrutura que contem o trinco de cada uma das contas
*	e destroi. Funcao apenas deve ser chamada depois de terminadas as tarefas, caso
*	contrario corre o risco de se bloquear.
*
*****************************************************************************************/

void trincos_contas_destroy() {

	/* Indice */
	int i;

	/* Percorre os Trincos das Contas */
	for( i=0; i <= NUM_CONTAS; i++ ) {

		/* Se foi Inicializado Destruimos */
		if ( trincos_contas[i].inicializado ) {
			pthread_mutex_destroy(&trincos_contas[i].mutex);
			trincos_contas[i].inicializado = 0;
		}

	}

}

/****************************************************************************************
*
*   2.1. INICIALIZA CONTAS
*
*****************************************************************************************/

void contas_init() {

	/* Indice */
	int i;

	for (i=0; i<NUM_CONTAS; i++) {
		contasSaldos[i] = 0;
	}

}

/****************************************************************************************
*
*   2.2. CONTA EXISTE
*
*****************************************************************************************/

int contaExiste(int idConta) {
	return (idConta > 0 && idConta <= NUM_CONTAS);
}

/****************************************************************************************
*
*   2.3. MENOR ID CONTA
*
*****************************************************************************************/

int menor(int idConta_a, int idConta_b) {
	if (idConta_a < idConta_b) {
		return idConta_a;
	} else {
		return idConta_b;
	}
}

/****************************************************************************************
*
*   2.4. MAIOR ID CONTA
*
*****************************************************************************************/

int maior(int idConta_a, int idConta_b) {
	if (idConta_a > idConta_b) {
		return idConta_a;
	} else {
		return idConta_b;
	}
}

/****************************************************************************************
*
*   2.3. DEBITAR
*
*****************************************************************************************/

int debitar(int idConta, int valor) {

	if (!contaExiste(idConta))
		return -1;
	if (contasSaldos[idConta - 1] < valor)
		return -1;
	atrasar();

	/* Guarda Valor de Retorno */
	int retrn;

	/* Cria trinco para conta se ainda nao foi criado */
	trinco_conta_init( idConta - 1 );

	/* Bloqueio alteracoes / acesso a esta conta */
	retrn = pthread_mutex_lock(&trincos_contas[idConta - 1].mutex);
	if ( retrn != 0 ) {
		printf("%s : Erro a bloquear o trinco da conta %d.\n", strerror(errno), idConta);
		exit(EXIT_FAILURE);
	}

	/* Operacao */
	contasSaldos[idConta - 1] -= valor;

	/* Desbloqueio alteracoes / acesso a esta conta */
	retrn = pthread_mutex_unlock(&trincos_contas[idConta - 1].mutex);
	if ( retrn != 0 ) {
		printf("%s : Erro a desbloquear o trinco da conta %d.\n", strerror(errno), idConta);
		exit(EXIT_FAILURE);
	}

	return 0;
}

/****************************************************************************************
*
*   2.4. CREDITAR
*
*****************************************************************************************/

int creditar(int idConta, int valor) {

	if (!contaExiste(idConta))
		return -1;

	/* Guarda Valor de Retorno */
	int retrn;

	/* Cria trinco para conta se ainda nao foi criado */
	trinco_conta_init( idConta - 1 );

	/* Bloqueio alteracoes / acesso a esta conta */
	retrn = pthread_mutex_lock(&trincos_contas[idConta - 1].mutex);
	if ( retrn != 0 ) {
		printf("%s : Erro a bloquear o trinco da conta %d.\n", strerror(errno), idConta);
		exit(EXIT_FAILURE);
	}

	/* Operacao */
	contasSaldos[idConta - 1] += valor;

	/* Desbloqueio alteracoes / acesso a esta conta */
	retrn = pthread_mutex_unlock(&trincos_contas[idConta - 1].mutex);
	if ( retrn != 0 ) {
		printf("%s : Erro a desbloquear o trinco da conta %d.\n", strerror(errno), idConta);
		exit(EXIT_FAILURE);
	}

	return 0;
}

/****************************************************************************************
*
*   2.5. LER SALDO
*
*****************************************************************************************/

int lerSaldo(int idConta) {

	/* Guarda saldo */
	int saldo;

	atrasar();
	if (!contaExiste(idConta))
		return -1;

	/* Guarda Valor de Retorno */
	int retrn;

	/* Cria trinco para conta se ainda nao foi criado */
	trinco_conta_init( idConta - 1 );

	/* Bloqueio alteracoes / acesso a esta conta */
	retrn = pthread_mutex_lock(&trincos_contas[idConta - 1].mutex);
	if ( retrn != 0 ) {
		printf("%s : Erro a bloquear o trinco da conta %d.\n", strerror(errno), idConta);
		exit(EXIT_FAILURE);
	}

	/* Operacao */
	saldo = contasSaldos[idConta - 1];

	/* Desbloqueio alteracoes / acesso a esta conta */
	retrn = pthread_mutex_unlock(&trincos_contas[idConta - 1].mutex);
	if ( retrn != 0 ) {
		printf("%s : Erro a desbloquear o trinco da conta %d.\n", strerror(errno), idConta);
		exit(EXIT_FAILURE);
	}

	return saldo;

}

/****************************************************************************************
*
*   2.6. TRANSFERIR
*
*****************************************************************************************/

int transferir(int idConta_a, int idConta_b, int valor) {

	if (!contaExiste(idConta_a) || !contaExiste(idConta_b))
		return -1;
	if (contasSaldos[idConta_a - 1] < valor )
		return -1;

	/* Guarda Valor de Retorno */
	int retrn;

	/* Cria trinco para contas se ainda nao foram criados */
	trinco_conta_init( idConta_a - 1);
	trinco_conta_init( idConta_b - 1);

	/* Bloqueio alteracoes / acesso a conta de origem */
	retrn = pthread_mutex_lock(&trincos_contas[menor(idConta_a-1,idConta_b-1)].mutex);
	if ( retrn != 0 ) {
		printf("%s : Erro a bloquear o trinco da conta %d.\n", strerror(errno), idConta_a);
		exit(EXIT_FAILURE);
	}

	/* Bloqueio alteracoes / acesso a conta de destino */
	retrn = pthread_mutex_lock(&trincos_contas[maior(idConta_a-1,idConta_b-1)].mutex);
	if ( retrn != 0 ) {
		printf("%s : Erro a bloquear o trinco da conta %d.\n", strerror(errno), idConta_b);
		exit(EXIT_FAILURE);
	}

	/* Debito do valor em A & Credito do valor em B  */
	contasSaldos[idConta_a - 1] -= valor;
	contasSaldos[idConta_b - 1] += valor;

	/* Desbloqueio alteracoes / acesso a conta de destino */
	retrn = pthread_mutex_unlock(&trincos_contas[maior(idConta_a-1,idConta_b-1)].mutex);
	if ( retrn != 0 ) {
		printf("%s : Erro a desbloquear o trinco da conta %d.\n", strerror(errno), idConta_b);
		exit(EXIT_FAILURE);
	}

	/* Desbloqueio alteracoes / acesso a conta de origem */
	retrn = pthread_mutex_unlock(&trincos_contas[menor(idConta_a-1,idConta_b-1)].mutex);
	if ( retrn != 0 ) {
		printf("%s : Erro a desbloquear o trinco da conta %d.\n", strerror(errno), idConta_a);
		exit(EXIT_FAILURE);
	}

	return 0;

}

/****************************************************************************************
*
*   2.7. SIMULAR
*
*****************************************************************************************/

void simular(int numAnos) {

	/* Dados simulacao */
	int ano;
	int saldo;
	int nr_conta;

	/* Buffer e ficheiro de registo */
	char nome_ficheiro[50];
	int fd, retrn;

	/* Constroi nome do ficheiro de output */
	sprintf(nome_ficheiro, "i-banco-sim-%d.txt", getpid());

	/* Cria / abre ficheiro para guardar output */
	fd = open(nome_ficheiro, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR );
	if ( fd == -1 ) {
		printf("%s : Erro a criar / ou abrir ficheiro simulacao.\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* Redireciona output */
	dup2(fd, 1);

	/* Para cada ano */
	for ( ano=0; ano <= numAnos && !(sair_agora==1); ano++ ) {

		printf("SIMULACAO: Ano %d\n=================\n", ano);

		/* Por cada Conta */
		for ( nr_conta=1; nr_conta <= NUM_CONTAS; nr_conta++ ) {

			/* Permite Verificao Execucao em Backgroud */
			atrasar();

			if (ano > 0) {
				saldo = lerSaldo(nr_conta);
				creditar(nr_conta, saldo * TAXAJURO);
				saldo = lerSaldo(nr_conta);
				debitar(nr_conta, (CUSTOMANUTENCAO > saldo) ? saldo : CUSTOMANUTENCAO);
			}

			saldo = lerSaldo(nr_conta);

			/* Mostra saldo (c/ tratamento de possiveis interrupcoes) */
			while ( printf("Conta %d, Saldo %d \n", nr_conta, lerSaldo(nr_conta) ) < 0) {
				if (errno == EINTR) {
					continue;
				} else {
					break;
				}
			}

		}

		printf("\n");

		/* Se queremos 'sair agora' terminamos programa */
		if ( sair_agora == 1 ) {

			/* Fecha ficheiro */
			retrn = close(fd);
			if ( retrn == -1 ) {
				printf("%s : Erro a fechar ficheiro simulacao.\n", strerror(errno));
			}

			exit(EXIT_FAILURE);
		}

	}

	/* Fecha ficheiro */
	retrn = close(fd);
	if ( retrn == -1 ) {
		printf("%s : Erro a fechar ficheiro simulacao.\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

}