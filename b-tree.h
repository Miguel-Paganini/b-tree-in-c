#ifndef B_TREE_H
#define B_TREE_H

#define ORDEM 5
#define MAX_NOME 100
#define MAX_TEL  20
#define ARQUIVO_REGISTROS "registros.csv"
#define ARQUIVO_ARVORE    "arvore.txt"

/* ---------------------------------------------------------
 * Estruturas
 * --------------------------------------------------------- */
typedef struct _chave {
    int  valor;  /* matrícula – chave de busca */
    long pos;    /* offset em bytes no arquivo de registros */
} Chave;

typedef struct _pagina {
    int qtd;
    Chave chaves[ORDEM - 1];
    struct _pagina *filho[ORDEM];
    int folha;
} Pagina;

typedef Pagina* ArvoreB;

/* ---------------------------------------------------------
 * Criação de páginas / árvore
 * --------------------------------------------------------- */
Pagina  *criaPagina(int folha);
ArvoreB  criarArvB(void);

/* ---------------------------------------------------------
 * Busca
 * --------------------------------------------------------- */
int buscaArvB(Pagina *pagAtual, int chave,
              Pagina **pagChave, int *posChave);

/* ---------------------------------------------------------
 * Inserção
 * --------------------------------------------------------- */
void    splitPagina(int chaveInserida, int posicaoFilho,
                    Pagina *pagina, int *chavePromovida,
                    Pagina **paginaFilhoDireita, Pagina **pagNova);
int     insereArvB(Pagina *pagAtual, int chave,
                   int *chavePromovida, Pagina **pagPromovida);
Pagina *inserir(Pagina *raiz, int chave);

/* ---------------------------------------------------------
 * Arquivo de registros
 * --------------------------------------------------------- */
long    gravarRegistro(const char *nomeArq, int matricula,
                       const char *nome, const char *tel);
int     lerRegistro(const char *nomeArq, long pos,
                    int *matricula, char *nome, char *tel);
Pagina *carregarRegistros(const char *nomeArq);

/* ---------------------------------------------------------
 * Persistência e liberação da árvore
 * --------------------------------------------------------- */
void gravarArvore(Pagina *raiz, const char *nomeArq);
void liberarArvore(Pagina *raiz);

#endif /* B_TREE_H */
