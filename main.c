#include <stdio.h>
#include <stdlib.h>
#include "b-tree.h"

int main() {
    // Cria a raiz (não folha)
    ArvoreB raiz = criaPagina(0);

    // Preenche a raiz com algumas chaves
    raiz->qtd = 2;
    raiz->chaves[0].valor = 10;
    raiz->chaves[1].valor = 20;

    // Cria filhos da raiz
    raiz->filho[0] = criaPagina(1);
    raiz->filho[1] = criaPagina(1);
    raiz->filho[2] = criaPagina(1);

    // Filho 0 (valores menores que 10)
    raiz->filho[0]->qtd = 2;
    raiz->filho[0]->chaves[0].valor = 1;
    raiz->filho[0]->chaves[1].valor = 5;

    // Filho 1 (entre 10 e 20)
    raiz->filho[1]->qtd = 2;
    raiz->filho[1]->chaves[0].valor = 12;
    raiz->filho[1]->chaves[1].valor = 15;

    // Filho 2 (maiores que 20)
    raiz->filho[2]->qtd = 2;
    raiz->filho[2]->chaves[0].valor = 25;
    raiz->filho[2]->chaves[1].valor = 30;

    // Teste de busca
    Pagina pagEncontrada;
    int pos;

    int chaveBusca = 15;

    if (buscaArvB(raiz, chaveBusca, &pagEncontrada, &pos)) {
        printf("Chave %d encontrada na posicao %d da pagina!\n", chaveBusca, pos);
    } else {
        printf("Chave %d nao encontrada.\n", chaveBusca);
    }

    return 0;
}
