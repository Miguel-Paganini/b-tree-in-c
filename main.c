#include <stdio.h>
#include <stdlib.h>
#include "b-tree.h"

int main() {
    // Cria a raiz (não folha)
    ArvoreB arv = criarArvB();
    
    arv = inserir(arv, 14);
    arv = inserir(arv, 15);
    arv = inserir(arv, 16);
    arv = inserir(arv, 17);
    

    // Teste de busca
    Pagina *pagEncontrada;
    int pos;

    for(int chaveBusca = 14; chaveBusca < 18; chaveBusca++){
        if (buscaArvB(arv, chaveBusca, &pagEncontrada, &pos)) {
            printf("Chave %d encontrada na posicao %d da pagina!\n", chaveBusca, pos);
        } else {
            printf("Chave %d nao encontrada.\n", chaveBusca);
        }
    }

    return 0;
}
