#ifndef BTREE_H
#define BTREE_H

#define ORDEM 4

typedef struct _chave {
    int valor;
    int pos;  // índice do registro no arquivo (acesso direto)
} Chave;

typedef struct _pagina {
    int qtd;
    Chave chaves[ORDEM - 1];
    struct _pagina *filho[ORDEM];
    int folha;
} Pagina;

typedef Pagina *ArvoreB;

ArvoreB  criarArvB();
Pagina  *criaPagina(int folha);
int      buscaArvB(Pagina *pagAtual, int chave, Pagina **pagChave, int *posChave);

/* Parâmetros adicionados: posInserida e posPromovida para propagar o campo pos da Chave */
void     splitPagina(int chaveInserida, int posInserida, int posicaoFilho,
                     Pagina *pagina,
                     int *chavePromovida, int *posPromovida,
                     Pagina **paginaFilhoDireita, Pagina **pagNova);

int      insereArvB(Pagina *pagAtual, int chave, int pos,
                    int *chavePromovida, int *posPromovida,
                    Pagina **pagPromovida);

Pagina  *inserir(Pagina *raiz, int chave);           // mantido para compatibilidade
Pagina  *inserirComPos(Pagina *raiz, int chave, int pos);  // novo: usa pos real do arquivo

#endif