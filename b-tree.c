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
    return criaPagina(1);
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