#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "b-tree.h"

/* =========================================================
 * CRIAÇÃO
 * ========================================================= */

Pagina *criaPagina(int folha)
{
    Pagina *pagina = (Pagina*) malloc(sizeof(Pagina));
    if (!pagina) return NULL;

    pagina->qtd   = 0;
    pagina->folha = folha;

    for (int i = 0; i < ORDEM - 1; i++) {
        pagina->chaves[i].valor = 0;
        pagina->chaves[i].pos   = 0;
    }
    for (int i = 0; i < ORDEM; i++)
        pagina->filho[i] = NULL;

    return pagina;
}

ArvoreB criarArvB(void)
{
    return criaPagina(0);
}


/* =========================================================
 * BUSCA
 * ========================================================= */

int buscaArvB(Pagina *pagAtual, int chave,
              Pagina **pagChave, int *posChave)
{
    if (!pagAtual) return 0;

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


/* =========================================================
 * INSERÇÃO
 * ========================================================= */

void splitPagina(int chaveInserida, int posicaoFilho,
                 Pagina *pagina, int *chavePromovida,
                 Pagina **paginaFilhoDireita, Pagina **pagNova)
{
    *pagNova = criaPagina(pagina->folha);
    if (!*pagNova) return;

    /* cópias temporárias com espaço para a nova chave/filho */
    Chave  chavesTemp[ORDEM];
    Pagina *filhosTemp[ORDEM + 1];

    for (int i = 0; i < pagina->qtd; i++) {
        chavesTemp[i]  = pagina->chaves[i];
        filhosTemp[i]  = pagina->filho[i];
    }
    filhosTemp[pagina->qtd] = pagina->filho[pagina->qtd];

    /* abre espaço para inserir */
    for (int i = pagina->qtd; i > posicaoFilho; i--) {
        chavesTemp[i]     = chavesTemp[i - 1];
        filhosTemp[i + 1] = filhosTemp[i];
    }
    chavesTemp[posicaoFilho].valor = chaveInserida;
    filhosTemp[posicaoFilho + 1]   = *paginaFilhoDireita;

    int meio = ORDEM / 2;
    *chavePromovida    = chavesTemp[meio].valor;
    *paginaFilhoDireita = *pagNova;

    /* metade esquerda fica na página original */
    for (int i = 0; i < meio; i++) {
        pagina->chaves[i] = chavesTemp[i];
        pagina->filho[i]  = filhosTemp[i];
    }
    pagina->filho[meio] = filhosTemp[meio];
    pagina->qtd = meio;

    /* metade direita vai para a página nova */
    for (int i = meio + 1; i < ORDEM; i++) {
        (*pagNova)->chaves[i - meio - 1] = chavesTemp[i];
        (*pagNova)->filho[i - meio - 1]  = filhosTemp[i];
    }
    (*pagNova)->filho[ORDEM - 1 - meio] = filhosTemp[ORDEM];
    (*pagNova)->qtd = ORDEM - 1 - meio;
}

int insereArvB(Pagina *pagAtual, int chave,
               int *chavePromovida, Pagina **pagPromovida)
{
    if (!pagAtual) {
        *chavePromovida = chave;
        *pagPromovida   = NULL;
        return 1;
    }

    int i = 0;
    while (i < pagAtual->qtd && chave > pagAtual->chaves[i].valor)
        i++;

    if (i < pagAtual->qtd && chave == pagAtual->chaves[i].valor) {
        printf("ERRO: chave %d já existe na árvore.\n", chave);
        return -1;
    }

    int retorno = insereArvB(pagAtual->filho[i], chave,
                             chavePromovida, pagPromovida);
    if (retorno != 1) return retorno;

    if (pagAtual->qtd < ORDEM - 1) {
        /* há espaço: insere diretamente */
        for (int j = pagAtual->qtd; j > i; j--) {
            pagAtual->chaves[j]     = pagAtual->chaves[j - 1];
            pagAtual->filho[j + 1]  = pagAtual->filho[j];
        }
        pagAtual->chaves[i].valor = *chavePromovida;
        pagAtual->filho[i + 1]    = *pagPromovida;
        pagAtual->qtd++;
        return 0;
    }

    /* página cheia: split */
    Pagina *pagNova = NULL;
    splitPagina(*chavePromovida, i, pagAtual,
                chavePromovida, pagPromovida, &pagNova);
    return 1;
}

Pagina *inserir(Pagina *raiz, int chave)
{
    int    chavePromovida;
    Pagina *pagPromovida = NULL;

    int ret = insereArvB(raiz, chave, &chavePromovida, &pagPromovida);
    if (ret == 1) {
        Pagina *novaRaiz = criaPagina(0);
        novaRaiz->chaves[0].valor = chavePromovida;
        novaRaiz->filho[0]        = raiz;
        novaRaiz->filho[1]        = pagPromovida;
        novaRaiz->qtd             = 1;
        return novaRaiz;
    }
    return raiz;
}


/* =========================================================
 * ARQUIVO DE REGISTROS
 *
 * Formato do registros.csv:
 *   <matricula>,<nome>,<telefone>\n
 *
 * O campo `pos` de cada chave da árvore guarda o offset
 * em bytes (ftell) do início da linha correspondente,
 * permitindo acesso direto via fseek – sem varredura.
 * ========================================================= */

long gravarRegistro(const char *nomeArq, int matricula,
                    const char *nome, const char *tel)
{
    FILE *f = fopen(nomeArq, "a");
    if (!f) {
        perror("Erro ao abrir arquivo de registros");
        return -1;
    }

    fseek(f, 0, SEEK_END);
    long pos = ftell(f);

    fprintf(f, "%d,%s,%s\n", matricula, nome, tel);
    fclose(f);
    return pos;
}

int lerRegistro(const char *nomeArq, long pos,
                int *matricula, char *nome, char *tel)
{
    FILE *f = fopen(nomeArq, "r");
    if (!f) {
        perror("Erro ao abrir arquivo de registros");
        return 0;
    }

    fseek(f, pos, SEEK_SET);   /* acesso direto */

    char linha[256];
    if (!fgets(linha, sizeof(linha), f)) {
        fclose(f);
        return 0;
    }
    fclose(f);

    char *tok = strtok(linha, ",");
    if (!tok) return 0;
    *matricula = atoi(tok);

    tok = strtok(NULL, ",");
    if (!tok) return 0;
    strncpy(nome, tok, MAX_NOME - 1);
    nome[MAX_NOME - 1] = '\0';

    tok = strtok(NULL, "\n");
    if (!tok) return 0;
    strncpy(tel, tok, MAX_TEL - 1);
    tel[MAX_TEL - 1] = '\0';

    return 1;
}

Pagina *carregarRegistros(const char *nomeArq)
{
    Pagina *raiz = criarArvB();

    FILE *f = fopen(nomeArq, "r");
    if (!f) return raiz;   /* arquivo ainda não existe */

    char linha[256];
    while (1) {
        long pos = ftell(f);
        if (!fgets(linha, sizeof(linha), f)) break;

        if (linha[0] == '\n' || linha[0] == '\r' || linha[0] == '\0')
            continue;

        char copia[256];
        strncpy(copia, linha, sizeof(copia));

        char *tok = strtok(copia, ",");
        if (!tok) continue;
        int matricula = atoi(tok);
        if (matricula <= 0) continue;

        int    chavePromovida;
        Pagina *pagPromovida = NULL;
        int ret = insereArvB(raiz, matricula, &chavePromovida, &pagPromovida);

        if (ret == 1) {
            Pagina *novaRaiz = criaPagina(0);
            novaRaiz->chaves[0].valor = chavePromovida;
            novaRaiz->filho[0]        = raiz;
            novaRaiz->filho[1]        = pagPromovida;
            novaRaiz->qtd             = 1;
            raiz = novaRaiz;
        }

        /* associa o offset à chave inserida */
        if (ret >= 0) {
            Pagina *pChave = NULL;
            int     posC   = -1;
            if (buscaArvB(raiz, matricula, &pChave, &posC))
                pChave->chaves[posC].pos = pos;
        }
    }

    fclose(f);
    return raiz;
}


/* =========================================================
 * PERSISTÊNCIA DA ÁRVORE
 * ========================================================= */

void gravarArvore(Pagina *raiz, const char *nomeArq)
{
    FILE *f = fopen(nomeArq, "w");
    if (!f) {
        perror("Erro ao criar arquivo da árvore");
        return;
    }

    /* primeira linha: endereço do nó raiz */
    fprintf(f, "RAIZ=%p\n", (void*)raiz);

    if (!raiz) { fclose(f); return; }

    /* BFS para gravar todos os nós em largura */
    Pagina *fila[4096];
    int ini = 0, fim = 0;
    fila[fim++] = raiz;

    while (ini < fim) {
        Pagina *pag = fila[ini++];
        if (!pag) continue;

        fprintf(f, "NO=%p qtd=%d folha=%d",
                (void*)pag, pag->qtd, pag->folha);
        for (int i = 0; i < pag->qtd; i++)
            fprintf(f, " chave[%d]={val=%d,pos=%ld}",
                    i, pag->chaves[i].valor, pag->chaves[i].pos);
        fprintf(f, "\n");

        for (int i = 0; i <= pag->qtd; i++)
            if (pag->filho[i] && fim < 4096)
                fila[fim++] = pag->filho[i];
    }

    fclose(f);
    printf("Árvore gravada em \"%s\".\n", nomeArq);
}


/* =========================================================
 * LIBERAÇÃO DE MEMÓRIA
 * ========================================================= */

void liberarArvore(Pagina *raiz)
{
    if (!raiz) return;
    for (int i = 0; i <= raiz->qtd; i++)
        liberarArvore(raiz->filho[i]);
    free(raiz);
}
