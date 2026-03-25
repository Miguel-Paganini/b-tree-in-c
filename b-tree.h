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

Pagina *criaPagina(int folha);
int buscaArvB(Pagina *pagAtual, int chave, Pagina **pagChave, int *posChave);
