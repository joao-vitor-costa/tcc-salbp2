// salbp2project.cpp : Este arquivo contém a função 'main'. A execução do programa começa e termina ali.
//


// SALBP2.cpp : define o ponto de entrada para o aplicativo do console.

/*
Nome: João Vitor Almeida Costa
Discente em Sistemas de Informação
Orientador: Dayan de Castro Bissoli
Atualizado em: 25/03/2019

Descrição: Desenvolver uma meta-heurística
para resolver o problema de balanceamento de
linha de montagem simples, onde se refere à
atribuição de tarefas com tempos de processamento
pré-definidos a estações de trabalho que estão
dispostas em uma linha, respeitando-se as restrições
de precedência entre as tarefas. Com objetivo de
minimizar o tempo de ciclo para um número
fixo de estações de trabalho.

*/


#include "pch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <iostream>


using namespace std;
//variaveis da solucao
#pragma warning (disable : 4996)
#define MAXTAREFA 29
#define MAXESTACAO 8
#define MAXPREDECENTE 100000


//Variaveis do SA
#define A 0.975
#define TC 0.01
#define SAmax 1000
float T0 = 1000;
float tempoSA;

typedef struct Tarefa {
	int id;
	int vetpredecesor[MAXTAREFA]; // Vetor onde irá conter todas os id das tarefas predecessoras. 
	int custo;
	int controlaPredecessor; // flag que retorna o numero de predecessores aquela tarefa possui.
}tarefa;

typedef struct Predecedente {
	int idTafera;      // id da terafa
	int idPredecedente; // predecedente da tarefa
}predecedente;

typedef struct Solucao {
	int matrizSolucao[MAXESTACAO][MAXTAREFA];
	int vetControlaNumeroTarefa[MAXESTACAO]; // Cada posição do vetor será o id -1 da estação para controle
	int vetCustoEstacao[MAXESTACAO]; // Cada posição do vetor será o id -1 da estação para controle
	int Fo;
}solucao;


// VARIAVEIS GLOBAIS
static int numMaxTarefa;
tarefa vetTarefa[MAXTAREFA];
static predecedente vetPredecedente[MAXPREDECENTE];
static int controlaNUmPrecedente; // Usada na leitura dos dados
int ultimaEstacaoInvivel;



/* Esse metodo ler as entradas e armazana nas estruturas criadas.*/
void lerDados() {
	FILE* f = fopen("Data/Entradas/BUXEY.IN2", "r");
	if (f != NULL) {
		fscanf(f, "%d", &numMaxTarefa);
		for (int i = 0; i < numMaxTarefa; i++) {
			fscanf(f, "%d", &vetTarefa[i].custo);
			vetTarefa[i].id = i + 1;
		}

		int i = 0;
		while (!feof(f)) {
			fscanf(f, "%d %d", &vetPredecedente[i].idPredecedente, &vetPredecedente[i].idTafera);
			i++;
		}
		controlaNUmPrecedente = i - 2;

	}
	else {
		cout << "Não Foi possivel abrir o arquivo." << endl;
	}
	fclose(f);
}


void limpaControladorFo(solucao &s) {
	for (int i = 0; i < MAXESTACAO; i++) {
		s.vetControlaNumeroTarefa[i] = 0;
		s.vetCustoEstacao[i] = 0;
	}
}

void sanitizarMatrizSolucao(solucao &s) {
	for (int i = 0; i < MAXESTACAO; i++) {
		for (int j = 0; j < MAXTAREFA; j++) {
			s.Fo = 0;
			s.matrizSolucao[i][j] = -1;
			s.vetControlaNumeroTarefa[i] = 0;
			s.vetCustoEstacao[i] = 0;
		}
	}
}

/*Esse metodo é utilizado para zerar os vetores globais*/
void preProcessamento(solucao &s) {
	for (int i = 0; i < numMaxTarefa; i++) {
		vetTarefa[i].controlaPredecessor = 0;

	}

	for (int i = 0; i < numMaxTarefa; i++) {
		for (int j = 0; j < numMaxTarefa; j++) {
			vetTarefa[i].vetpredecesor[j] = 0;
		}
	}

	limpaControladorFo(s);

}


/*Metodo realizar a atribuição dos id de cada predecessor ao vetor de predecessor de uma tarefa*/
void processarPredecedente(solucao &s) {
	preProcessamento(s);
	int idtarefa, idpredecedente, j;
	for (int i = 0; i < controlaNUmPrecedente; i++) {
		idpredecedente = vetTarefa[vetPredecedente[i].idTafera - 1].controlaPredecessor;
		idtarefa = vetPredecedente[i].idTafera - 1;
		j = vetTarefa[idtarefa].controlaPredecessor;
		vetTarefa[idtarefa].vetpredecesor[j] = vetPredecedente[i].idPredecedente;
		vetTarefa[idtarefa].controlaPredecessor = vetTarefa[vetPredecedente[i].idTafera - 1].controlaPredecessor + 1;
	}

}

/*Relatorio dos dados lidos*/
void gravarInfoDadosTarefas() {
	FILE* f = fopen("Data/infoTarefa.txt", "w");
	if (f != NULL) {
		fprintf(f, "\n##################################### NUMERO MAXIMO DE POSTOS #####################################\n");
		fprintf( f,"\t\t\t\t\t\t %d", MAXESTACAO);
		fprintf(f, "\n##################################### NUMERO MAXIMO DE TAREFAS #####################################\n");
		fprintf(f, "\t\t\t\t\t\t %d",numMaxTarefa);
		fprintf(f, "\n##################################### CUSTO DAS TAREFAS #####################################\n");
		int custo = 0;
		for (int i = 0; i < numMaxTarefa; i++) {
			fprintf(f, "\t\t\t\t\t tarefa %d = %d\n",vetTarefa[i].id, vetTarefa[i].custo);
			custo = custo + vetTarefa[i].custo;
		}
		fprintf(f, "\n##################################### CUSTO TOTAL: %d #####################################\n", custo);

		fprintf(f, "\n##################################### PREDECEDENTES #####################################\n");
		for (int i = 0; i < numMaxTarefa; i++) {
			for (int j = 0; j < vetTarefa[i].controlaPredecessor; j++) {
				fprintf(f, "\t\t\t\tTarefa = %d -  Predecessor = %d \n",vetTarefa[i].id, vetTarefa[i].vetpredecesor[j]);
			}


		}
		fclose(f);
	}
}


/*Relatorio dos dados lidos*/
void infoDadosTarefas() {
	cout << "##################################### NUMERO MAXIMO DE POSTOS #####################################" << endl;
	cout << "\t\t\t\t\t\t" << MAXESTACAO << endl;
	cout << "##################################### NUMERO MAXIMO DE TAREFAS #####################################" << endl;
	cout << "\t\t\t\t\t\t" << numMaxTarefa << endl;
	cout << "##################################### CUSTO DAS TAREFAS #####################################" << endl;
	int custo = 0;
	for (int i = 0; i < numMaxTarefa; i++) {
		cout << "\t\t\t\ttarefa " << vetTarefa[i].id << " = " << vetTarefa[i].custo << endl;
		custo = custo + vetTarefa[i].custo;
	}
	cout << "##################################### CUSTO TOTAL: " << custo << " #####################################" << endl;

	cout << "##################################### PREDECEDENTES #####################################" << endl;
	for (int i = 0; i < numMaxTarefa; i++) {
		for (int j = 0; j < vetTarefa[i].controlaPredecessor; j++) {
			cout << "\t\t\t\tTarefa = " << vetTarefa[i].id << " -  Predecessor = " << vetTarefa[i].vetpredecesor[j] << endl;
		}


	}
}


/*calculo do tempo de ciclo entre as estações de trabalho*/
void calculaFo(solucao &s) {
	s.Fo = 0;
	for (int i = 0; i < MAXESTACAO; i++) {
		if (s.Fo < s.vetCustoEstacao[i]) {
			s.Fo = s.vetCustoEstacao[i];
		}

	}
}

/*Metodo recebe um id da tarefa e verifica em quais estações de trabalho ela não pode entrar para
realizar a inserção, no fim do metodo ele cria uma lista de estações inviaveis para adicionar a tarefa*/
void verificarViabilidadeEstacoes(int idTarefa, solucao &s) {
	cout << endl;
	cout << endl;
	cout << "##################################### ESTACAOS DE TRABALHO INVIAVEIS #####################################" << endl;
	for (int k = 0; k < MAXESTACAO; k++) {
		for (int i = 0; i < s.vetControlaNumeroTarefa[k]; i++) {
			for (int j = 0; j < vetTarefa[idTarefa].controlaPredecessor; j++) {
				if (s.matrizSolucao[k][i] == vetTarefa[idTarefa].vetpredecesor[j]) { // comparo para ver se a tarefa que esta na matriz solução é igual a predecessor
					ultimaEstacaoInvivel = k;

				}
			}
		}
	}
	cout << "ID DA TAREFA " << idTarefa + 1 << " NAO PODE ENTRAR EM ESTACOES MENORES A " << ultimaEstacaoInvivel + 1 << endl;
	cout << "#########################################################################################################" << endl;

}

int sortearEstacao() {
	return rand() % (MAXESTACAO);
}


int sortearAleatorioEstacaoViavel() {
	cout << endl;
	cout << endl;
	cout << "##################################### SORTEANDO ESTACAO DE TRABALHO VIAVEIS ################################" << endl;
	int idEstacao;
	idEstacao = sortearEstacao();
	/*O loop da idEstacao ser menor que a ultima inviavel me permite ter a possibilidade de inserir
	a tarefa no final da estação inviviavel, lembrando que a tarefa não pode começar antes do seu predecessor
	por tando para inserir, ela pode entrar na ultima estação inviavel ou em estações acima*/
	while (idEstacao < ultimaEstacaoInvivel) {
		idEstacao = sortearEstacao();
	}

	return idEstacao;
	cout << "ESTACAO SORTEADA: " << idEstacao + 1 << endl;
	cout << "########################################################################################################" << endl;


}


void zerarEstacaoInviaveis() {
	ultimaEstacaoInvivel = -1;
}

/*Gero um solução totalmente aleatoria, respeitando a restrição do problema*/
void gerarConstrutivaAleatoria(solucao &s) {
	cout << endl;
	cout << endl;
	cout << "##################################### INCIANDO GERACAO ALEATORIA #####################################" << endl;
	int idtarefa, idEstacao;
	for (int i = 0; i < numMaxTarefa; i++) {
		cout << endl;
		cout << endl;
		cout << "##################################### INICIO DA INSERCAO TAREFA " << i + 1 << " ##################################### " << endl;
		idtarefa = i;
		zerarEstacaoInviaveis();
		verificarViabilidadeEstacoes(idtarefa, s);
		if (ultimaEstacaoInvivel == -1) { // posso colocar em qualquer estação de trabalho
			cout << "PARA TAREFA " << i + 1 << " NAO EXISTE NO MOMENTO POSTOS INVIAVEIS." << endl;
			idEstacao = sortearEstacao(); // Escolho uma aleatoriamente  para add a tarefa
			cout << "TAREFA " << i + 1 << " SERA INSERIDA NO POSTO DE TRABALHO " << idEstacao + 1 << endl;
		}
		else {
			idEstacao = sortearAleatorioEstacaoViavel();
			cout << "SORTEANDO POSTO PARA INSERIR A TAREFA" << endl;
			cout << "ATENCAO TAREFA ATUAL SO PODE ENTRAR NOS POSTO MAIORES QUE O POSTO " << ultimaEstacaoInvivel << endl;
			cout << "POSTO DE TAREFA SORTEADO " << idEstacao + 1 << " IRA RECEBER A TAREFA " << i + 1 << endl;
		}
		//Atribuo a tarefa a estação selecionada
		s.matrizSolucao[idEstacao][s.vetControlaNumeroTarefa[idEstacao]] = vetTarefa[i].id;
		s.vetControlaNumeroTarefa[idEstacao] = s.vetControlaNumeroTarefa[idEstacao] + 1;
		s.vetCustoEstacao[idEstacao] = s.vetCustoEstacao[idEstacao] + vetTarefa[i].custo;
		cout << "#####################################  FIM DA INSERCAO DA TAREFA " << i + 1 << " ##################################### " << endl;

	}
	calculaFo(s);
	zerarEstacaoInviaveis();
	cout << "##################################### FIM DA GERACAO ALEATORIA ##################################### " << endl;

}




/*Imprimo num relatorio da matriz solucao, mostrando as estações de trabalho e sua tarefas e o custo de cada estação*/
void imprimirSolucao(solucao &s) {
	for (int i = 0; i < MAXESTACAO; i++) {
		cout << endl;
		cout << endl;
		cout << "ESTACAO DE TRABALHO " << i + 1 << " POSSUI UM CUSTO TOTAL: " << s.vetCustoEstacao[i] << endl;
		cout << "TOTAL DE TAREFAS " << s.vetControlaNumeroTarefa[i] << endl;
		cout << "TAREFAS: ";
		if (s.vetControlaNumeroTarefa[i] > 0) {
			for (int j = 0; j < s.vetControlaNumeroTarefa[i]; j++) {
				cout << s.matrizSolucao[i][j] << " | ";

			}

			cout << endl;

		}
	}
	cout << endl;
	cout << endl;
	cout << "FO = " << s.Fo << endl;
	cout << endl;

}


void gravarResultado(solucao &s) {
	FILE* f = fopen("Data/Resultado.txt", "a");
	if (f == NULL) {
		f = fopen("Data/Resultado.txt", "w");
	}
	else {
		for (int i = 0; i < MAXESTACAO; i++) {
			fprintf(f, "\n");
			fprintf(f, "\n");
			fprintf(f, "ESTACAO DE TRABALHO %d POSSUI UM CUSTO TOTAL: %d\n", i + 1, s.vetCustoEstacao[i]);
			fprintf(f, "\n");
			fprintf(f, "TOTAL DE TAREFAS: \n", s.vetControlaNumeroTarefa[i]);
			fprintf(f, "TAREFAS: ");
			if (s.vetControlaNumeroTarefa[i] > 0) {
				for (int j = 0; j < s.vetControlaNumeroTarefa[i]; j++) {
					fprintf(f, "%d", s.matrizSolucao[i][j]);
					fprintf(f, " | ");

				}
				fprintf(f, "\n");

			}
		}
		fprintf(f, "\n");
		fprintf(f, "\n");
		fprintf(f, "FO = %d", s.Fo);
		fprintf(f, "\n");
		fclose(f);
	}

}



/*
Metodo responsavel de me direcionar para qual lado devo ir.
A primeira estação de trabalho sempre irá andar para direita
e a ultima estacao de trabalho sempre irá andar para esquerda.
-1 - para esquerda
1 - para direita*/
int sortearDirecao(int idTarefa) {
	int direcao;
	idTarefa = idTarefa + 1;
	if (idTarefa == 1) {
		return 1;
	}

	if (idTarefa == MAXESTACAO) {
		return -1;
	}

	direcao = rand() % (2);

	if (direcao == 0) {
		return -1;
	}

	return direcao;
}

/*Esse metodo quando retiro uma tarefa a uma estação eu chamo ele para atualizar as varias de controle*/
void subtrairTarefaCustoEContador(solucao &s, int idEstacao, int idtarefa) {
	int custoAtual, custoNovo, custoTarefa, controlador;
	custoAtual = s.vetCustoEstacao[idEstacao];
	custoTarefa = vetTarefa[idtarefa].custo;
	custoNovo = custoAtual - custoTarefa;
	s.vetCustoEstacao[idEstacao] = custoNovo;
	controlador = s.vetControlaNumeroTarefa[idEstacao];
	s.vetControlaNumeroTarefa[idEstacao] = controlador - 1;
}

/*Esse metodo quando adiciono uma tarefa a uma estação eu chamo ele para atualizar as varias de controle*/
void somarTarefaCustoEContador(solucao &s, int idEstacao, int idtarefa) {
	int custoAtual, custoNovo, custoTarefa, controlador;
	custoAtual = s.vetCustoEstacao[idEstacao];
	custoTarefa = vetTarefa[idtarefa].custo;
	s.vetCustoEstacao[idEstacao] = custoAtual + custoTarefa;
	controlador = s.vetControlaNumeroTarefa[idEstacao];
	s.vetControlaNumeroTarefa[idEstacao] = controlador + 1;
}

/*Realizo apenas um troca de posições da estação onde foi removida a tarefa*/
void trocarPosicaoEstacaoVelha(solucao &s, int idEstacaoVelha, int idTarefaSorteada) {
	int auxTempTarefa;
	for (int i = idTarefaSorteada; i + 1 < s.vetControlaNumeroTarefa[idEstacaoVelha]; i++) {
		auxTempTarefa = s.matrizSolucao[idEstacaoVelha][i + 1];
		s.matrizSolucao[idEstacaoVelha][i] = auxTempTarefa;
	}
}

/*Me retorna a ultima posição inviavel de uma estação que eu posso inserir*/
int validarPosicaoNova(solucao &s, int idEstacaoNova, int idTarefa) {
	/*rever os maximo  e colocar em variaceis para debugar*/
	int predecessor;
	int ultimaPosicaoInViavel = -1;
	int maximoTarefa = s.vetControlaNumeroTarefa[idEstacaoNova];
	for (int i = 0; i < maximoTarefa; i++) {
		int tarefa = s.matrizSolucao[idEstacaoNova][i];
		int maximoPredecessor = vetTarefa[idTarefa].controlaPredecessor;
		for (int j = 0; j < maximoPredecessor; j++) {
			predecessor = vetTarefa[idTarefa].vetpredecesor[j];
			if (predecessor == tarefa) {
				ultimaPosicaoInViavel = i;
			}
		}
	}
	return ultimaPosicaoInViavel;

}


/*Metodo responsavel de receber um posicao e organizar a matriz para eu inserir numa posição especifica
é inserido na posicao + 1*/
void trocarPosicaoEstacaoNova(solucao &s, int idEstacaoNova, int idtarefa, int posicao) {
	int tarefaAux, temp;
	tarefaAux = s.matrizSolucao[idEstacaoNova][posicao];
	s.matrizSolucao[idEstacaoNova][posicao] = idtarefa;
	for (int i = posicao; i + 1 < s.vetControlaNumeroTarefa[idEstacaoNova]; i++) {
		temp = s.matrizSolucao[idEstacaoNova][i + 1];
		s.matrizSolucao[idEstacaoNova][i + 1] = tarefaAux;
		tarefaAux = temp;
	}

}

/*Esse metodo irá inserir na estação ja escolhida, irá inserir no final caso eu possa inserir em qualquer lugar
e caso eu tenha um posicao especifica eu chamo um metodo que irá realizar essa troca de posicao*/
void inserirTarefaEstacaoNova(solucao &s, int idEstacaoNova, int idtarefa) {
	int posicao = validarPosicaoNova(s, idEstacaoNova, idtarefa - 1);
	if (posicao == -1) {// Posso inserir em qualquer lugar, insiro na ultima posicao
		s.matrizSolucao[idEstacaoNova][s.vetControlaNumeroTarefa[idEstacaoNova]] = idtarefa;
	}
	else {
		/*Insiro numa posição N+1 da ultima tarefa que é seu predecessor*/
		trocarPosicaoEstacaoNova(s, idEstacaoNova, idtarefa, posicao);
	}
	int idVetTarefa = idtarefa - 1;
	somarTarefaCustoEContador(s, idEstacaoNova, idVetTarefa);

}

/*Esse metodo irá inserir uma tarefa nova na estação que eu escolhi*/
void inserirTarefaViavelNaEstacao(solucao &s, int idEstacaoVelha, int idEstacaoNova, int idTarefaSorteada) {
	int auxIdtarefa, tempIdtarefa;
	if (idEstacaoNova != idEstacaoVelha) {
		auxIdtarefa = s.matrizSolucao[idEstacaoVelha][idTarefaSorteada] - 1; // representa o id na no vetor de tarefas
		tempIdtarefa = s.matrizSolucao[idEstacaoVelha][idTarefaSorteada]; // o dia da tarefa a nivel de leitura 1 a MAXTAREFA
		cout << "TAREFA QUE IRA SE MUDAR PARA OUTRA VIZINHO " << auxIdtarefa + 1 << endl;
		/*Se entrar nesse if, quer dizer que a tareda que quero mover é a ultima*/
		if (idTarefaSorteada + 1 >= s.vetControlaNumeroTarefa[idEstacaoVelha]) {
			s.matrizSolucao[idEstacaoVelha][idTarefaSorteada] = -1;
			subtrairTarefaCustoEContador(s, idEstacaoVelha, auxIdtarefa);
		}
		else {
			trocarPosicaoEstacaoVelha(s, idEstacaoVelha, idTarefaSorteada);
			subtrairTarefaCustoEContador(s, idEstacaoVelha, auxIdtarefa);
		}
		/*Nesse ponto a estaçãoVelha já não possui a tarefa que foi sorteada, proximo passo é inserir a tarefa no
		novo posto de trabalho, respeitando a viabilidade da solucao*/
		inserirTarefaEstacaoNova(s, idEstacaoNova, tempIdtarefa);
	}

}

/*Garanto que não estou sorteando uma estação vazia, pois a estação sorteada deve conter pelo menos
uma tarefa, já que eu vou retirar a tarefa da estação.*/
int sortearEstacaoNaoVazia(solucao &s) {
	int idEstacaoSorteada = sortearEstacao();
	int maximoNumeroTarefa = s.vetControlaNumeroTarefa[idEstacaoSorteada];
	while (maximoNumeroTarefa <= 0) {
		idEstacaoSorteada = sortearEstacao();
		maximoNumeroTarefa = s.vetControlaNumeroTarefa[idEstacaoSorteada];
	}

	return idEstacaoSorteada;
}

/*Sorteio uma tarefa que será mudada para outra estação de trabalho*/
int sortearTarefa(solucao &s, int idEstacaoSorteada) {
	int maximoNumeroTarefa = s.vetControlaNumeroTarefa[idEstacaoSorteada];
	int idTarefaSorteada = rand() % (maximoNumeroTarefa);
	return idTarefaSorteada;
}



/*
O metodo gerarVizinhoMovimentoEsquerdaDireita ele irá sortear uma direção para qual irá movimentar
a tarefa, podendo levar a tarefa para estação N-1 ou N+1, para validar a inserção verifico qual é
ultima estação inviavel que poss inserir a tarefa sorteada e vejo se a estação N-1 ou N+1 é menor
que a ultima estação inviavel, caso for quer dizer que estou deixando a solução inviavel
então com um loop gero novamente os valores até que eu insira de forma que não quebre
a viabilidade das taredas
*/
void gerarVizinhoMovimentoEsquerdaDireita(solucao &s) {
	int idEstacaoNova, direcao, idEstacaoSorteada, idTarefaSorteada;
	idEstacaoSorteada = sortearEstacaoNaoVazia(s); // sorteio uma estação de trabalho
	direcao = sortearDirecao(idEstacaoSorteada); // sorteio uma direção para para esquerda ou para direita
	idTarefaSorteada = sortearTarefa(s, idEstacaoSorteada); // pego uma tarefa sorteada que ira sair da estação, posso pegar qualquer tarefa, desda primeira a ultima
	zerarEstacaoInviaveis(); // zero a varivel global
	verificarViabilidadeEstacoes(s.matrizSolucao[idEstacaoSorteada][idTarefaSorteada] - 1, s); // vejo se a estação sorteada pode receber essa tarefa, verificando se nao quebra viabilidade da solução
	idEstacaoNova = idEstacaoSorteada + direcao; // sorteio a nova estação, se direção for esquerda irá subtrair 1, se for direira irar somar 1
	if (ultimaEstacaoInvivel != -1) { // Se for -1 posso colocar onde quiser.
		while ((ultimaEstacaoInvivel != -1) || (idEstacaoNova < ultimaEstacaoInvivel)) { // verifico o caso de eu estiver selecionado uma estação que esteja atrasr de um estação inviavel, como a linha é sequencial só posso pegar estações viaveis, por exemplo, se tenho 4 estações e a ultima inviavel é a 2, posso usar a 3 ou 4 para ser a nova estação.
			idEstacaoSorteada = sortearEstacaoNaoVazia(s);
			direcao = sortearDirecao(idEstacaoSorteada);
			idTarefaSorteada = sortearTarefa(s, idEstacaoSorteada);
			zerarEstacaoInviaveis();
			verificarViabilidadeEstacoes(s.matrizSolucao[idEstacaoSorteada][idTarefaSorteada] - 1, s);
			idEstacaoNova = idEstacaoSorteada + direcao;
		}
	}
	cout << "A TAREFA " << s.matrizSolucao[idEstacaoSorteada][idTarefaSorteada] << " NA ESTACAO " << idEstacaoSorteada + 1 << " VAI SE MUDAR PARA ESTACAO " << idEstacaoNova + 1 << endl;
	inserirTarefaViavelNaEstacao(s, idEstacaoSorteada, idEstacaoNova, idTarefaSorteada);
}


/*Metodo totalmente aleatorio, escolho uma estação aleatoriamente, escolho uma tarefa aleatoria dessa estação,
e verifico qual é aultima estação inviavel que posso adicionar a tarefa, ai sorteo uma nova estação que ira receber
essa tarefa.*/
void gerarVizinhoTotalmenteAleatorio(solucao &s) {
	int idEstacaoNova, idEstacaoSorteada, idTarefaSorteada;
	idEstacaoSorteada = sortearEstacaoNaoVazia(s); // sorteio uma estação de trabalho
	idTarefaSorteada = sortearTarefa(s, idEstacaoSorteada); // pego uma tarefa sorteada que ira sair da estação, posso pegar qualquer tarefa, desda primeira a ultima
	zerarEstacaoInviaveis(); // zero a varivel global
	verificarViabilidadeEstacoes(s.matrizSolucao[idEstacaoSorteada][idTarefaSorteada] - 1, s); // vejo se a estação sorteada pode receber essa tarefa, verificando se nao quebra viabilidade da solução
	idEstacaoNova = rand() % (MAXESTACAO);
	while ((ultimaEstacaoInvivel != -1) || (idEstacaoNova < ultimaEstacaoInvivel)) { // verifico o caso de eu estiver selecionado uma estação que esteja atrasr de um estação inviavel, como a linha é sequencial só posso pegar estações viaveis, por exemplo, se tenho 4 estações e a ultima inviavel é a 2, posso usar a 3 ou 4 para ser a nova estação.
		idEstacaoSorteada = sortearEstacaoNaoVazia(s);
		idTarefaSorteada = sortearTarefa(s, idEstacaoSorteada);
		zerarEstacaoInviaveis();
		verificarViabilidadeEstacoes(s.matrizSolucao[idEstacaoSorteada][idTarefaSorteada] - 1, s);
		idEstacaoNova = rand() % (MAXESTACAO);
	}
	cout << "A TAREFA " << s.matrizSolucao[idEstacaoSorteada][idTarefaSorteada] << " NA ESTACAO " << idEstacaoSorteada + 1 << " VAI SE MUDAR PARA ESTACAO " << idEstacaoNova + 1 << endl;
	inserirTarefaViavelNaEstacao(s, idEstacaoSorteada, idEstacaoNova, idTarefaSorteada);
}



/*metodo irá clonar um solução em outra. Recebo um construção s e clono ela em p*/
void gerarClone(solucao &s, solucao &p) {
	memcpy(&p, &s, sizeof(s));
}

/*Pego uma solução S e pego o melhor vizinho dos seus N vizinhos, ese metedo repeti esse procidemento pegando a
melhor solução que achei no for de dentro e repete o processo, até eu sair com o melhor vizinho. Dentro do metodo
possuo duas gerações de vizinhaça que chama uma ou outra aleatoriamente, a soloção gerarVizinhoMovimentoEsquerdaDireita pega uma
estação e vê se vai mudar a tarefa selecionada, para maquina de traz(esquerda) ou para maquina da frente(direita). Já  a geração de
vizinho do gerarVizinhoTotalmenteAleatorio, manda uma tarefa aletoriamente para uma estação viavel, onde possa receber ela. A geração
de vizinhos respeita a regra de predecedencia das estações, logo só gero vizinhos validos e viaveis.
*/
void metodoMelhorVizinho(solucao &s) {
	solucao melhor;
	int temp, flag;
	gerarClone(s, melhor);

	//For externo que roda N vezes com a melhor solução atual
	for (int idMelhor = 0; idMelhor < 100; idMelhor++) {
		gerarClone(melhor, s);
		// for interno tenta pegar a melhor solução da solução X
		for (int i = 0; i < 100; i++) {

			int geravizinho = rand() % (2);

			if (geravizinho == 0) {
				gerarVizinhoMovimentoEsquerdaDireita(s);
			}
			else {
				gerarVizinhoTotalmenteAleatorio(s);
			}
			calculaFo(s);
			if (s.Fo <= melhor.Fo) {
				gerarClone(s, melhor);

			}
		}
	}
	calculaFo(s);
	gerarClone(melhor, s);
}


/*Estrutura de do SA --- Conversar com Dayan para vê se pode ter mudanças, nçao rodei o algoritmo, só montei para DAYAN validar a logica*/
void simulatedAnnealing(solucao &s) {
	clock_t hi = clock();
	float tempo = 0;
	int interT = 0;
	clock_t hf;
	solucao s1;
	solucao sVizinho;
	float delta;
	gerarClone(s, s1);
	float T = T0;

	while (tempo < 120) {

		while (T > TC) { // clausula de parada do tempo, TC temperatura congelante
			while (interT < SAmax) {
				interT++;
				gerarClone(s, sVizinho);

				int geravizinho = rand() % (2);

				if (geravizinho == 0) {
					gerarVizinhoMovimentoEsquerdaDireita(sVizinho);
				}
				else {
					gerarVizinhoTotalmenteAleatorio(sVizinho);
				}
				delta = sVizinho.Fo - s.Fo;
				if (delta < 0) {
					gerarClone(sVizinho, s);
					if (sVizinho.Fo < s1.Fo) {
						gerarClone(sVizinho, s1);
					}
				}
				else {
					float x = rand() % (INT_MAX);
					x = x / (INT_MAX - 1);
					if (x < exp(-delta / T)) {
						gerarClone(sVizinho, s);
					}
				}

			}
			hf = clock();
			tempo = (hf - hi) / CLOCKS_PER_SEC;
			interT = 0;

			T = T * A;
			if (tempo > 120) {
				break;
			}
		}
		T = T0;
	}
	hf = clock();
	tempoSA = (hf - hi) / CLOCKS_PER_SEC;
}





int main() {
	srand(time(0));
	solucao s;
	lerDados();
	processarPredecedente(s);
	float duracao = 0;
	clock_t si, sf;
	si = clock();
	sanitizarMatrizSolucao(s);
	gerarConstrutivaAleatoria(s);
	imprimirSolucao(s);
	metodoMelhorVizinho(s);
	sf = clock();
	duracao = ((sf - si) / CLOCKS_PER_SEC);
	system("cls");
	infoDadosTarefas();
	imprimirSolucao(s);	
	gravarInfoDadosTarefas();
	gravarResultado(s);
	cout << "TEMPO  " << duracao << " SEGUNDOS. " << endl;
	system("pause");
	return 0;
}


