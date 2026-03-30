#define ORDEM 4

/*
typedef struct _chave Chave;

typedef struct _pagina Pagina;
*/

typedef struct _chave {
    int valor;
    int pos;
} Chave;

typedef struct _pagina {
    int qtd;
    Chave chaves[ORDEM - 1];
    struct _pagina *filho[ORDEM];
    int folha;
} Pagina;

typedef Pagina *ArvoreB;

ArvoreB criarArvB();
Pagina *criaPagina(int folha);
int buscaArvB(Pagina *pagAtual, int chave, Pagina **pagChave, int *posChave);
void splitPagina(int chaveInserida, int posicaoFilho, Pagina *pagina, int *chavePromovida, Pagina **paginaFilhoDireita, Pagina **pagNova);
int insereArvB(Pagina *pagAtual, int chave, int *chavePromovida, Pagina **pagPromovida);
Pagina* inserir(Pagina *raiz, int chave);
