#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "b-tree.h"

/* ------------------------------------------------------------------ */
/*  Criação de páginas / árvore                                        */
/* ------------------------------------------------------------------ */

Pagina *criaPagina(int folha) {
    Pagina *pagina = (Pagina *) malloc(sizeof(Pagina));
    if (!pagina) return NULL;

    pagina->qtd  = 0;
    pagina->folha = folha;

    for (int i = 0; i < ORDEM - 1; i++) {
        pagina->chaves[i].valor = 0;
        pagina->chaves[i].pos  = 0;
    }
    for (int i = 0; i < ORDEM; i++)
        pagina->filho[i] = NULL;

    return pagina;
}

ArvoreB criarArvB() {
    return criaPagina(0);
}

/* ------------------------------------------------------------------ */
/*  Busca                                                               */
/* ------------------------------------------------------------------ */

int buscaArvB(Pagina *pagAtual, int chave, Pagina **pagChave, int *posChave) {
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

/* ------------------------------------------------------------------ */
/*  Split                                                               */
/*                                                                      */
/*  Adicionados: posInserida (pos da chave a inserir) e                */
/*               posPromovida (pos da chave que será promovida).        */
/*  Corrigidos:  filho[meio] da página esquerda e                       */
/*               filho[qtd] da página direita (pagNova).               */
/* ------------------------------------------------------------------ */

void splitPagina(int chaveInserida, int posInserida, int posicaoFilho,
                 Pagina *pagina,
                 int *chavePromovida, int *posPromovida,
                 Pagina **paginaFilhoDireita, Pagina **pagNova) {

    *pagNova = criaPagina(pagina->folha);
    if (!*pagNova) return;

    /* Copia tudo para arrays temporários */
    Chave   chavesTemp[ORDEM];
    Pagina *filhosTemp[ORDEM + 1];

    for (int i = 0; i < pagina->qtd; i++) {
        chavesTemp[i] = pagina->chaves[i];
        filhosTemp[i] = pagina->filho[i];
    }
    filhosTemp[pagina->qtd] = pagina->filho[pagina->qtd];

    /* Abre espaço e insere a nova chave nos temporários */
    for (int i = pagina->qtd; i > posicaoFilho; i--) {
        chavesTemp[i]     = chavesTemp[i - 1];
        filhosTemp[i + 1] = filhosTemp[i];
    }
    chavesTemp[posicaoFilho].valor = chaveInserida;
    chavesTemp[posicaoFilho].pos   = posInserida;      /* ← propaga pos */
    filhosTemp[posicaoFilho + 1]   = *paginaFilhoDireita;

    /* Divide ao meio */
    int meio = ORDEM / 2;

    *chavePromovida = chavesTemp[meio].valor;
    *posPromovida   = chavesTemp[meio].pos;            /* ← propaga pos */
    *paginaFilhoDireita = *pagNova;

    /* Página esquerda: chaves[0..meio-1], filho[0..meio] */
    for (int i = 0; i < meio; i++) {
        pagina->chaves[i] = chavesTemp[i];
        pagina->filho[i]  = filhosTemp[i];
    }
    pagina->filho[meio] = filhosTemp[meio];            /* ← filho direito do meio */
    pagina->qtd = meio;

    /* Página direita (pagNova): chaves[meio+1..ORDEM-1], filho[meio+1..ORDEM] */
    for (int i = meio + 1; i < ORDEM; i++) {
        (*pagNova)->chaves[i - meio - 1] = chavesTemp[i];
        (*pagNova)->filho[i - meio - 1]  = filhosTemp[i];
    }
    (*pagNova)->filho[ORDEM - 1 - meio] = filhosTemp[ORDEM]; /* ← filho mais direito */
    (*pagNova)->qtd = ORDEM - 1 - meio;
}

/* ------------------------------------------------------------------ */
/*  Inserção interna                                                    */
/*                                                                      */
/*  Adicionados: int pos (pos do registro no arquivo),                  */
/*               int *posPromovida (retorno da pos promovida).          */
/* ------------------------------------------------------------------ */

int insereArvB(Pagina *pagAtual, int chave, int pos,
               int *chavePromovida, int *posPromovida,
               Pagina **pagPromovida) {

    Pagina *pagNova = NULL;

    /* Caso base: chegou além de uma folha → promove a chave */
    if (pagAtual == NULL) {
        *chavePromovida = chave;
        *posPromovida   = pos;
        *pagPromovida   = NULL;
        return 1;
    }

    int i = 0;
    while (i < pagAtual->qtd && chave > pagAtual->chaves[i].valor)
        i++;

    if (i < pagAtual->qtd && chave == pagAtual->chaves[i].valor) {
        printf("ERRO: chave %d já existe na árvore.\n", chave);
        return -1;
    }

    int retorno = insereArvB(pagAtual->filho[i], chave, pos,
                             chavePromovida, posPromovida, pagPromovida);
    if (retorno != 1) return retorno;

    /* Há espaço na página atual → apenas insere */
    if (pagAtual->qtd < ORDEM - 1) {
        for (int j = pagAtual->qtd; j > i; j--) {
            pagAtual->chaves[j]    = pagAtual->chaves[j - 1];
            pagAtual->filho[j + 1] = pagAtual->filho[j];
        }
        pagAtual->chaves[i].valor = *chavePromovida;
        pagAtual->chaves[i].pos   = *posPromovida;   /* ← grava pos */
        pagAtual->filho[i + 1]    = *pagPromovida;
        pagAtual->qtd++;
        return 0;
    }

    /* Página cheia → split */
    splitPagina(*chavePromovida, *posPromovida, i,
                pagAtual,
                chavePromovida, posPromovida,
                pagPromovida, &pagNova);
    return 1;
}

/* ------------------------------------------------------------------ */
/*  Funções públicas de inserção                                        */
/* ------------------------------------------------------------------ */

/*
 * inserirComPos: versão principal — usa o índice real do registro no arquivo.
 * É chamada por carregarArvore() e cadastrar().
 */
Pagina *inserirComPos(Pagina *raiz, int chave, int pos) {
    int chavePromovida, posPromovida;
    Pagina *pagPromovida = NULL;

    int retorno = insereArvB(raiz, chave, pos,
                             &chavePromovida, &posPromovida, &pagPromovida);

    if (retorno == 1) {   /* raiz precisou ser dividida → cria nova raiz */
        Pagina *novaRaiz = criaPagina(0);
        novaRaiz->chaves[0].valor = chavePromovida;
        novaRaiz->chaves[0].pos   = posPromovida;
        novaRaiz->filho[0] = raiz;
        novaRaiz->filho[1] = pagPromovida;
        novaRaiz->qtd = 1;
        return novaRaiz;
    }

    return raiz;
}

/*
 * inserir: mantido por compatibilidade com o main.c original.
 * Usa pos=0 (não serve para acesso direto ao arquivo).
 */
Pagina *inserir(Pagina *raiz, int chave) {
    return inserirComPos(raiz, chave, 0);
}