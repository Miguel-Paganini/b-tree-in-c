#define ORDEM 8

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
ArvoreB criarArvB();

ArvoreB inserirArvB(ArvoreB raiz, Chave chave);
