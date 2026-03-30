#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "b-tree.h"

/*
struct _chave {
    int valor;
    int pos;
};

struct _pagina {
    int qtd;
    Chave chaves[ORDEM - 1];
    struct _pagina *filho[ORDEM];
    int folha;
};
*/

Pagina *criaPagina(int folha) {
    Pagina *pagina = (Pagina*) malloc(sizeof(Pagina));
    if (!pagina) return NULL;

    pagina->qtd = 0;
    pagina->folha = folha;

    for (int i=0; i < ORDEM-1; i++) {
        pagina->chaves[i].valor = 0;
        pagina->chaves[i].pos = 0;
    }

    for (int i=0; i < ORDEM; i++) {
        pagina->filho[i] = NULL;
    }

    return pagina;
}

ArvoreB criarArvB(){
    return criaPagina(0);
}

int buscaArvB(Pagina *pagAtual, int chave, Pagina **pagChave, int *posChave){
    if (pagAtual == NULL) return 0;

    int i = 0;

    while (i < pagAtual->qtd && chave > pagAtual->chaves[i].valor)
        i++;
    
    if (i < pagAtual->qtd && chave == pagAtual->chaves[i].valor) {
        *pagChave = pagAtual;
        *posChave = i;
        return 1;
    }

    return buscaArvB(pagAtual->filho[i], chave, pagChave, posChave);
}

void splitPagina(int chaveInserida, int posicaoFilho, Pagina *pagina, int *chavePromovida, Pagina **paginaFilhoDireita, Pagina **pagNova) {
    *pagNova = criaPagina(pagina->folha);
    if (pagNova == NULL) return;

    Chave chavesTemp[ORDEM];
    Pagina *filhosTemp[ORDEM + 1];

    for (int i = 0; i < pagina->qtd; i++) {
        chavesTemp[i] = pagina->chaves[i];
        filhosTemp[i] = pagina->filho[i];
    }
    filhosTemp[pagina->qtd] = pagina->filho[pagina->qtd];

    for (int i = pagina->qtd; i > posicaoFilho; i--) {
        chavesTemp[i] = chavesTemp[i - 1];
        filhosTemp[i + 1] = filhosTemp[i];
    }
    chavesTemp[posicaoFilho].valor = chaveInserida;
    filhosTemp[posicaoFilho + 1] = *paginaFilhoDireita;

    int meio = ORDEM / 2;
    *chavePromovida = chavesTemp[meio].valor;
    
    *paginaFilhoDireita = *pagNova;
    
    for (int i = 0; i < meio; i++) {
        pagina->chaves[i] = chavesTemp[i];
        pagina->filho[i] = filhosTemp[i];
    }
    pagina->qtd = meio;
    
    for (int i = meio + 1; i < ORDEM; i++) {
        (*pagNova)->chaves[i - meio - 1] = chavesTemp[i];
        (*pagNova)->filho[i - meio - 1] = filhosTemp[i];
    }
    (*pagNova)->qtd = ORDEM - 1 - meio;
}

int insereArvB(Pagina *pagAtual, int chave, int *chavePromovida, Pagina **pagPromovida) {
    int retorno;
    Pagina *pagNova = NULL;
    
    if(pagAtual == NULL) {
        *chavePromovida = chave;
        *pagPromovida = NULL;
        return 1;
    }

    int i = 0;

    while(i < pagAtual->qtd && chave > pagAtual->chaves[i].valor)
        i++;

    if(i < pagAtual->qtd && chave == pagAtual->chaves[i].valor){
        printf("ERROR: Chave já existe na árvore.\n");
        return -1;
    }
    
    retorno = insereArvB(pagAtual->filho[i], chave, chavePromovida, pagPromovida);
    if (retorno != 1) return retorno;

    else if(pagAtual->qtd < ORDEM - 1) {
        for (int j = pagAtual->qtd; j > i; j--) {
            pagAtual->chaves[j] = pagAtual->chaves[j-1];
            pagAtual->filho[j+1] = pagAtual->filho[j];
        }

        pagAtual->chaves[i].valor = *chavePromovida;
        pagAtual->filho[i+1] = *pagPromovida;
        pagAtual->qtd++;
        return 0;
    }

    else{
        splitPagina(*chavePromovida, i, pagAtual, chavePromovida, pagPromovida, &pagNova);
        return 1;
    }

}

Pagina* inserir(Pagina *raiz, int chave) {
    int chavePromovida;
    Pagina *pagPromovida = NULL;

    int retorno = insereArvB(raiz, chave, &chavePromovida, &pagPromovida);

    if (retorno == 1) {
        Pagina *novaRaiz = criaPagina(0);
        novaRaiz->chaves[0].valor = chavePromovida;
        novaRaiz->filho[0] = raiz;
        novaRaiz->filho[1] = pagPromovida;
        novaRaiz->qtd = 1;
        return novaRaiz;
    }

    return raiz;
}