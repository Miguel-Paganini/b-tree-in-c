#include <stdlib.h>
#include <string.h>
#include "b-tree.h"

Pagina *criarPagina(int folha) {
    Pagina *pagina = (Pagina*) malloc(sizeof(Pagina));
    if (!pagina) return NULL;
    
    pagina->qtd = 0;
    pagina->folha = folha;
    
    for(int i=0; i < ORDEM-1; i++) {
        pagina->chaves[i].valor = 0;
        pagina->chaves[i].pos = 0;
    }
    
    for(int i=0; i < ORDEM; i++) {
        pagina->filho[i] = NULL;
    }
    
    return pagina;
}

ArvoreB criarArvB(){
    return criarPagina(1);
}

ArvoreB inserirArvB(ArvoreB raiz, Chave chave) {
    
}

