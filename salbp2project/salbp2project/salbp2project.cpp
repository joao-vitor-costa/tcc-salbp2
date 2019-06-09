// salbp2project.cpp : Este arquivo contém a função 'main'. A execução do programa começa e termina ali.
//


// SALBP2.cpp : define o ponto de entrada para o aplicativo do console.

/*
Nome: João Vitor Almeida Costa
Discente em Sistemas de Informação
Orientador: Dayan de Castro Bissoli
Atualizado em: 07/06/2019

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
#define MAXESTACAO 14
#define MAXPREDECENTE 29
#define MAXMBL 5000
#define MAXGERACAO 1000
#define PENALIZACAO 500



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
	int viavel; // 1 - Viavel - 0 inviavel
	int estacaoMaiorCusto; // guardo o id da estação que tem maior  tempo de ciclo
}solucao;


// VARIAVEIS GLOBAIS
static int numMaxTarefa;
tarefa vetTarefa[MAXTAREFA];
static predecedente vetPredecedente[MAXPREDECENTE];
static int controlaNUmPrecedente; // Usada na leitura dos dados
int ultimaEstacaoInvivel;
int totalVizinhosSA;
int totalVizinhosSAAceitos;
int penalizacao = 0;

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
		fprintf(f, "\t\t\t\t\t\t %d", MAXESTACAO);
		fprintf(f, "\n##################################### NUMERO MAXIMO DE TAREFAS #####################################\n");
		fprintf(f, "\t\t\t\t\t\t %d", numMaxTarefa);
		fprintf(f, "\n##################################### CUSTO DAS TAREFAS #####################################\n");
		int custo = 0;
		for (int i = 0; i < numMaxTarefa; i++) {
			fprintf(f, "\t\t\t\t\t tarefa %d = %d\n", vetTarefa[i].id, vetTarefa[i].custo);
			custo = custo + vetTarefa[i].custo;
		}
		fprintf(f, "\n##################################### CUSTO TOTAL: %d #####################################\n", custo);

		fprintf(f, "\n##################################### PREDECEDENTES #####################################\n");
		for (int i = 0; i < numMaxTarefa; i++) {
			for (int j = 0; j < vetTarefa[i].controlaPredecessor; j++) {
				fprintf(f, "\t\t\t\tTarefa = %d -  Predecessor = %d \n", vetTarefa[i].id, vetTarefa[i].vetpredecesor[j]);
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

void ordenarPosto(solucao &s, int posto) {
	for (int i = 0; i < s.vetControlaNumeroTarefa[posto]; i++) {
		int max = vetTarefa[s.matrizSolucao[posto][i] - 1].controlaPredecessor;
		for (int k = 0; k < max; k++) {
			for (int j = 0; j < s.vetControlaNumeroTarefa[posto]; j++) {
				int idPredecessor = vetTarefa[s.matrizSolucao[posto][i] - 1].vetpredecesor[k];
				int tarefa = s.matrizSolucao[posto][j];
				if (tarefa == idPredecessor) {
					if (j > i) {
						int aux = s.matrizSolucao[posto][j];
						s.matrizSolucao[posto][j] = s.matrizSolucao[posto][i];
						s.matrizSolucao[posto][i] = aux;
					}
				}
			}
		}
	}

}
// Ordenar os postos de trabalhos que não quebram a viabilidade porem fica desordenado os postos
void ordenaSolucao(solucao &s) {
	int viabilidade = 1;
	for (int k = 0; k < MAXESTACAO; k++) {
		for (int i = 0; i < s.vetControlaNumeroTarefa[k]; i++) {
			int idtarefaObservada = s.matrizSolucao[k][i];
			int max = vetTarefa[s.matrizSolucao[k][i] - 1].controlaPredecessor;
			for (int j = 0; j < max; j++) {
				for (int e = 0; e < MAXESTACAO; e++) {
					for (int t = 0; t < s.vetControlaNumeroTarefa[e]; t++) {
						int idtarefa = s.matrizSolucao[e][t];
						int idPredecessor = vetTarefa[s.matrizSolucao[k][i] - 1].vetpredecesor[j];
						if (idPredecessor == idtarefa) {
							// situacao onde esta no mesmo posto o predecessor, porem ficou a tarefa ficou antes dele, a tarefa é ordenada
							if (e == k) {
								ordenarPosto(s, k);
							}
						}

					}
				}
			}



		}
	}
}

void validaSolucao(solucao &s) {
	s.viavel = 1;
	for (int i = 0; i < MAXESTACAO; i++) {
		for (int j = 0; j < s.vetControlaNumeroTarefa[i]; j++) {
			int tarefaObservadora = s.matrizSolucao[i][j];
			int idVetTarefas = tarefaObservadora - 1;
			int maxPredecessor = vetTarefa[idVetTarefas].controlaPredecessor;
			for (int p = 0; p < maxPredecessor; p++) {
				int predecessorObservado = vetTarefa[idVetTarefas].vetpredecesor[p];
				for (int c = 0; c < MAXESTACAO; c++) {
					for (int l = 0; l < s.vetControlaNumeroTarefa[c]; l++) {
						int tarefaObservada = s.matrizSolucao[c][l];
						if (predecessorObservado == tarefaObservada) {
							if (c > i) {
								s.viavel = 0;
								break;
							}
						}
					}
				}
			}
		}
	}
}


void penalizaSolucao(solucao &s) {
	if (s.viavel == 0) {
		s.Fo = s.Fo + PENALIZACAO;
	}
}

/*calculo do tempo de ciclo entre as estações de trabalho*/
void calculaFo(solucao &s) {
	s.Fo = 0;
	for (int i = 0; i < MAXESTACAO; i++) {
		if (s.Fo < s.vetCustoEstacao[i]) {
			s.Fo = s.vetCustoEstacao[i];
			s.estacaoMaiorCusto = i;
		}
	}

	validaSolucao(s);
	penalizaSolucao(s);
}



/*Metodo recebe um id da tarefa e verifica em quais estações de trabalho ela não pode entrar para
realizar a inserção, no fim do metodo ele cria uma lista de estações inviaveis para adicionar a tarefa*/
void verificarViabilidadeEstacoes(int idTarefa, solucao &s) {
	//cout << endl;
	//cout << endl;
	//cout << "##################################### ESTACAOS DE TRABALHO INVIAVEIS #####################################" << endl;
	for (int k = 0; k < MAXESTACAO; k++) {
		for (int i = 0; i < s.vetControlaNumeroTarefa[k]; i++) {
			for (int j = 0; j < vetTarefa[idTarefa].controlaPredecessor; j++) {
				if (s.matrizSolucao[k][i] == vetTarefa[idTarefa].vetpredecesor[j]) { // comparo para ver se a tarefa que esta na matriz solução é igual a predecessor
					ultimaEstacaoInvivel = k;

				}
			}
		}
	}
	//	cout << "ID DA TAREFA " << idTarefa + 1 << " NAO PODE ENTRAR EM ESTACOES MENORES A " << ultimaEstacaoInvivel + 1 << endl;
	//	cout << "#########################################################################################################" << endl;

}

int sortearEstacao() {
	return rand() % (MAXESTACAO);
}


int sortearAleatorioEstacaoViavel() {
	//cout << endl;
	//cout << endl;
	//cout << "##################################### SORTEANDO ESTACAO DE TRABALHO VIAVEIS ################################" << endl;
	int idEstacao;
	idEstacao = sortearEstacao();
	/*O loop da idEstacao ser menor que a ultima inviavel me permite ter a possibilidade de inserir
	a tarefa no final da estação inviviavel, lembrando que a tarefa não pode começar antes do seu predecessor
	por tando para inserir, ela pode entrar na ultima estação inviavel ou em estações acima*/
	while (idEstacao < ultimaEstacaoInvivel) {
		idEstacao = sortearEstacao();
	}

	return idEstacao;
	//cout << "ESTACAO SORTEADA: " << idEstacao + 1 << endl;
	//cout << "########################################################################################################" << endl;


}


void zerarEstacaoInviaveis() {
	ultimaEstacaoInvivel = -1;
}

/*Gero um solução totalmente aleatoria, respeitando a restrição do problema*/
void gerarConstrutivaAleatoriaViavel(solucao &s) {
	//cout << endl;
	//cout << endl;
	//cout << "##################################### INCIANDO GERACAO ALEATORIA #####################################" << endl;
	int idtarefa, idEstacao;
	for (int i = 0; i < numMaxTarefa; i++) {
		//cout << endl;
		//cout << endl;
		//cout << "##################################### INICIO DA INSERCAO TAREFA " << i + 1 << " ##################################### " << endl;
		idtarefa = i;
		zerarEstacaoInviaveis();
		verificarViabilidadeEstacoes(idtarefa, s);
		if (ultimaEstacaoInvivel == -1) { // posso colocar em qualquer estação de trabalho
			//cout << "PARA TAREFA " << i + 1 << " NAO EXISTE NO MOMENTO POSTOS INVIAVEIS." << endl;
			idEstacao = sortearEstacao(); // Escolho uma aleatoriamente  para add a tarefa
			//cout << "TAREFA " << i + 1 << " SERA INSERIDA NO POSTO DE TRABALHO " << idEstacao + 1 << endl;
		}
		else {
			idEstacao = sortearAleatorioEstacaoViavel();
			//cout << "SORTEANDO POSTO PARA INSERIR A TAREFA" << endl;
			//cout << "ATENCAO TAREFA ATUAL SO PODE ENTRAR NOS POSTO MAIORES QUE O POSTO " << ultimaEstacaoInvivel << endl;
			//cout << "POSTO DE TAREFA SORTEADO " << idEstacao + 1 << " IRA RECEBER A TAREFA " << i + 1 << endl;
		}
		//Atribuo a tarefa a estação selecionada
		s.matrizSolucao[idEstacao][s.vetControlaNumeroTarefa[idEstacao]] = vetTarefa[i].id;
		s.vetControlaNumeroTarefa[idEstacao] = s.vetControlaNumeroTarefa[idEstacao] + 1;
		s.vetCustoEstacao[idEstacao] = s.vetCustoEstacao[idEstacao] + vetTarefa[i].custo;
		//cout << "#####################################  FIM DA INSERCAO DA TAREFA " << i + 1 << " ##################################### " << endl;

	}
	calculaFo(s);
	zerarEstacaoInviaveis();
	//cout << "##################################### FIM DA GERACAO ALEATORIA ##################################### " << endl;

}

void gerarConstrutivaAleatoria(solucao &s) {
	int idEstacao;
	for (int i = 0; i < numMaxTarefa; i++) {
		idEstacao = sortearEstacao();
		s.matrizSolucao[idEstacao][s.vetControlaNumeroTarefa[idEstacao]] = vetTarefa[i].id;
		s.vetControlaNumeroTarefa[idEstacao] = s.vetControlaNumeroTarefa[idEstacao] + 1;
		s.vetCustoEstacao[idEstacao] = s.vetCustoEstacao[idEstacao] + vetTarefa[i].custo;
	}
	calculaFo(s);

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
	if (s.viavel == 1) {
		cout << "SOLUCAO VIAVEL" << endl;
	}
	else {
		cout << "SOLUCAO INVIAVEL" << endl;
	}
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
		if (s.viavel == 1) {
			fprintf(f, "SOLUÇÃO VIAVEL");
		}
		else {
			fprintf(f, "SOLUÇÃO INVIAVEL");
		}
		fprintf(f, "\n");
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
		//cout << "TAREFA QUE IRA SE MUDAR PARA OUTRA VIZINHO " << auxIdtarefa + 1 << endl;
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



/*Realizado testes e não gerou bons resultados - NAO USAR*/
void gerarVizinhoMovimentoEsquerdaDireitaViavel(solucao &s) {
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
	//cout << "A TAREFA " << s.matrizSolucao[idEstacaoSorteada][idTarefaSorteada] << " NA ESTACAO " << idEstacaoSorteada + 1 << " VAI SE MUDAR PARA ESTACAO " << idEstacaoNova + 1 << endl;
	inserirTarefaViavelNaEstacao(s, idEstacaoSorteada, idEstacaoNova, idTarefaSorteada);
	calculaFo(s);
}


/*Realizado testes e não gerou bons resultados - NAO USAR*/
void gerarVizinhoTotalmenteAleatorioViavel(solucao &s) {
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
	//cout << "A TAREFA " << s.matrizSolucao[idEstacaoSorteada][idTarefaSorteada] << " NA ESTACAO " << idEstacaoSorteada + 1 << " VAI SE MUDAR PARA ESTACAO " << idEstacaoNova + 1 << endl;
	inserirTarefaViavelNaEstacao(s, idEstacaoSorteada, idEstacaoNova, idTarefaSorteada);
	calculaFo(s);
}


/*Metodo totalmente aleatorio, escolho uma estação aleatoriamente, escolho uma tarefa aleatoria dessa estação,
 e pego uma estação aleatoria para essa nova tarefa, esse metodo pode gerar vizinhos viaveis e inviaveis*/
void gerarVizinhoAleatorio(solucao &s) {
	int idEstacaoNova, idEstacaoVelha, idTarefaSorteada, auxIdtarefa, tempIdtarefa;;

	idEstacaoNova = rand() % (MAXESTACAO);
	idEstacaoVelha = sortearEstacaoNaoVazia(s); // sorteio uma estação de trabalho
	while (idEstacaoNova == idEstacaoVelha) {
		idEstacaoVelha = sortearEstacaoNaoVazia(s);
	}
	idTarefaSorteada = sortearTarefa(s, idEstacaoVelha);

	auxIdtarefa = s.matrizSolucao[idEstacaoVelha][idTarefaSorteada] - 1; // representa o id na no vetor de tarefas
	tempIdtarefa = s.matrizSolucao[idEstacaoVelha][idTarefaSorteada]; // o id da tarefa a nivel de leitura 1 a MAXTAREFA

	/*Se entrar nesse if, quer dizer que a tareda que quero mover é a ultima*/
	if (idTarefaSorteada + 1 >= s.vetControlaNumeroTarefa[idEstacaoVelha]) {
		s.matrizSolucao[idEstacaoVelha][idTarefaSorteada] = -1;
		subtrairTarefaCustoEContador(s, idEstacaoVelha, auxIdtarefa);
	}
	else {
		int auxTempTarefa;
		for (int i = idTarefaSorteada; i + 1 < s.vetControlaNumeroTarefa[idEstacaoVelha]; i++) {
			auxTempTarefa = s.matrizSolucao[idEstacaoVelha][i + 1];
			s.matrizSolucao[idEstacaoVelha][i] = auxTempTarefa;
		}
		s.matrizSolucao[idEstacaoVelha][s.vetControlaNumeroTarefa[idEstacaoVelha]] = -1;
		subtrairTarefaCustoEContador(s, idEstacaoVelha, auxIdtarefa);
	}
	/*Nesse ponto a estaçãoVelha já não possui a tarefa que foi sorteada, proximo passo é inserir a tarefa no
	novo posto de trabalho*/

	s.matrizSolucao[idEstacaoNova][s.vetControlaNumeroTarefa[idEstacaoNova]] = tempIdtarefa;
	somarTarefaCustoEContador(s, idEstacaoNova, auxIdtarefa);
	calculaFo(s);
}



/*metodo irá clonar um solução em outra. Recebo um construção s e clono ela em p*/
void gerarClone(solucao &s, solucao &p) {
	memcpy(&p, &s, sizeof(s));
}

/*Retorna o id da tarefa que possui a tarefa como predecedente*/
int retornaEstacaoDisponivel(int idTarefa, solucao &s) {
	int id;
	int tarefa = numMaxTarefa;
	for (int i = 0; i < numMaxTarefa; i++) {
		for (int j = 0; j < vetTarefa[i].controlaPredecessor; j++) {
			if (vetTarefa[i].vetpredecesor[j] == idTarefa) {
				id = vetTarefa[i].id;
				if (id <= tarefa) {
					tarefa = id;
				}
			}
		}
	}
	int retorno;
	for (int i = 0; i < MAXESTACAO; i++) {
		for (int j = 0; j < MAXTAREFA; j++) {
			if (s.matrizSolucao[i][j] == tarefa) {
				retorno = i;
			}
		}

	}
	return retorno;
}


void movimentoGuloso(solucao &s) {
	Solucao melhor;
	gerarClone(s, melhor);
	int custo = 0;
	int idtarefa;
	int parada = 0;

	int id = rand() % (melhor.vetControlaNumeroTarefa[melhor.estacaoMaiorCusto]);
	idtarefa = melhor.matrizSolucao[melhor.estacaoMaiorCusto][id];
	custo = vetTarefa[idtarefa - 1].custo;
	verificarViabilidadeEstacoes(idtarefa - 1, melhor);
	int estacaonova;
	int menorCusto = penalizacao;
	if (ultimaEstacaoInvivel == -1) {
		estacaonova = retornaEstacaoDisponivel(idtarefa, melhor);
		if (melhor.estacaoMaiorCusto == estacaonova - 1) {
			parada = 1;
		}
	}
	else {
		for (int i = 0; i <= ultimaEstacaoInvivel; i++) {
			if (melhor.vetCustoEstacao[i] <= menorCusto) {
				menorCusto = melhor.vetCustoEstacao[i];
				estacaonova = i;
			}
		}
	}

	int custoNovo = melhor.vetCustoEstacao[estacaonova] + custo;
	int custoAntigo = melhor.Fo - custo;
	if (custoNovo < custoAntigo) {
		parada = 0;
	}
	else {
		parada = 1;
	}

	if (parada == 0) {
		melhor.matrizSolucao[estacaonova][melhor.vetControlaNumeroTarefa[estacaonova]] = idtarefa;
		somarTarefaCustoEContador(melhor, estacaonova, idtarefa - 1);
		int auxTempTarefa;
		for (int i = id; i + 1 < melhor.vetControlaNumeroTarefa[melhor.estacaoMaiorCusto]; i++) {
			auxTempTarefa = melhor.matrizSolucao[melhor.estacaoMaiorCusto][i + 1];
			melhor.matrizSolucao[melhor.estacaoMaiorCusto][i] = auxTempTarefa;
		}
		melhor.matrizSolucao[melhor.estacaoMaiorCusto][melhor.vetControlaNumeroTarefa[melhor.estacaoMaiorCusto]] = -1;
		subtrairTarefaCustoEContador(melhor, melhor.estacaoMaiorCusto, idtarefa - 1);
		calculaFo(melhor);
		gerarClone(melhor, s);
	}
}


void metodoBucaLocal(solucao &s) {
	solucao melhor;
	int temp, flag;
	gerarClone(s, melhor);
	//For externo que roda N vezes com a melhor solução atual
	for (int idMelhor = 0; idMelhor < MAXMBL; idMelhor++) {
		gerarClone(melhor, s);
		// for interno tenta pegar a melhor solução da solução X
		for (int i = 0; i < MAXGERACAO; i++) {
			gerarVizinhoAleatorio(s);
			if (s.Fo <= melhor.Fo) {
				gerarClone(s, melhor);
			}
		}
	}
	calculaFo(melhor);
	gerarClone(melhor, s);
}

float temperaturaInicial(solucao &s, float beta, float gama, int SAmax, int T0) {
	Solucao sVizinho;
	float T = T0; // Temperatura Corrente
	int delta = 0;
	int condicao = 0;
	int iterT = 0;
	gerarClone(s, sVizinho);
	float duracao = 0;
	while (condicao == 0) {
		int aceitos = 0; // Número de vizinhos aceitos na temperatura T
		for (int i = 0; i < SAmax; i++) {
			gerarConstrutivaAleatoriaViavel(s);

			delta = sVizinho.Fo - s.Fo;
			if (delta < 0) {
				aceitos = aceitos + 1;
			}
			else {
				float x = rand() % 1001;
				x = x / 999;
				if (x < exp(-delta / T)) {
					aceitos = aceitos + 1;
				}
			}
		}

		if (aceitos >= gama * SAmax) {
			condicao = 1;
		}
		else {
			T = T * beta;
		}
	}
	T0 = T;
	return T0;
}



void simulatedAnnealing(solucao &s, float alfa, float TC, int SAmax, float T0) {
	Solucao sMelhor, sVizinho;
	int iterT = 0;
	float T = T0;
	int delta = 0;
	gerarClone(s, sMelhor);
	gerarClone(s, sVizinho);
	float duracao = 0;
	clock_t si, sf;
	si = clock();
	while (T > TC) {
		while (iterT < SAmax) {
			iterT = iterT + 1;
			gerarVizinhoAleatorio(s);
			delta = sVizinho.Fo - s.Fo;
			if (delta <= 0) {
				gerarClone(sVizinho, s);
				if (sVizinho.Fo <= sMelhor.Fo) {
					metodoBucaLocal(sVizinho);
					gerarClone(sVizinho, sMelhor);
				}
			}
			else {
				float x = rand() % 1001;
				x = x / 999;
				if (x < exp(-delta / T)) {
					gerarClone(sVizinho, s);
				}
			}
		}
		T = alfa * T;
		iterT = 0;

		sf = clock();
		duracao = ((sf - si) / CLOCKS_PER_SEC);

	}
	gerarClone(sMelhor, s);
}


int main() {
	srand(time(0));

	// Solução
	solucao s;
	//Variaveis do SA
	float alfa = 0.975;
	//float alfa = 0.995;
	float TC = 0.01;
	//float TC = 0.001;
	int SAmax = 1000000;
	float T0 = 2; // Temperatura inicial T0 nao calibrada


	//variaveis para calibrar T0
	float beta = 1.1; // Taxa de aumento da temperatura T0
	float gama = 0.90; // taxa mínima de aceitação de soluções vizinhas em porcentagem


	// Lendo as entradas
	lerDados();
	// Processa a entrada
	processarPredecedente(s);
	// Normaliza as soluções para serem geradas
	sanitizarMatrizSolucao(s);
	//Imprimi no console informações das tarefas
	infoDadosTarefas();
	//Grava em txt informações das tarefas
	gravarInfoDadosTarefas();
	// variaveis de tempo
	float duracao = 0;
	clock_t si, sf;
	si = clock();


	cout << "Processando da Geracao Aleatoria..." << endl;
	cout << endl;
	cout << " ################## SOLUCAO ALETORIA ###################" << endl;
	gerarConstrutivaAleatoriaViavel(s);
	//gerarConstrutivaAleatoria(s);
	ordenaSolucao(s);
	imprimirSolucao(s);
	gravarResultado(s);
	cout << endl;
	cout << "Fim do Processando da Geracao Aleatoria..." << endl;
	cout << endl;


	/*
	cout << "Processando Refinamento..." << endl;
	cout << endl;
	metodoBucaLocal(s);
	cout << " ################## SOLUCAO DO REFINAMENTO ###################" << endl;
	ordenaSolucao(s);
	imprimirSolucao(s);
	gravarResultado(s);
	cout << endl;
	cout << " Fim do processando Refinamento..." << endl;
	cout << endl;

	//	  */


		  // /*
	cout << "Calibrando T0 ..." << endl;
	cout << endl;
	T0 = temperaturaInicial(s, beta, gama, SAmax, T0);
	cout << "T0 = " << T0 << endl;
	cout << endl;
	//  */


	  /*


	cout << "Processando SA..." << endl;
	cout << endl;
	simulatedAnnealing(s, alfa, TC, SAmax, T0);
	cout << " ################## SOLUCAO DO SA ###################" << endl;
	imprimirSolucao(s);
	gravarResultado(s);
	cout << endl;
	cout << "Fim do Processamento do SA..." << endl;


	// */
	sf = clock();
	duracao = ((sf - si) / CLOCKS_PER_SEC);
	cout << "TEMPO  " << duracao << " SEGUNDOS. " << endl;
	system("pause");
	return 0;
}


