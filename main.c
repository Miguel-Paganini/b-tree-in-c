#include <stdio.h>
#include "b-tree.h"

int main(void)
{
    printf("Carregando registros de \"%s\"...\n", ARQUIVO_REGISTROS);
    Pagina *raiz = carregarRegistros(ARQUIVO_REGISTROS);
    printf("Árvore B carregada em memória.\n\n");

    executarMenu(&raiz);

    return 0;
}
