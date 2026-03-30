#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "b-tree.h"

/* ------------------------------------------------------------------ */
/*  Estrutura do registro gravado em arquivo binário                   */
/*                                                                      */
/*  Registro de tamanho FIXO → acesso direto por índice:               */
/*      fseek(fp, indice * sizeof(Registro), SEEK_SET)                 */
/* ------------------------------------------------------------------ */

#define TAM_NOME 50
#define TAM_TEL  15

typedef struct {
    int  matricula;
    char nome[TAM_NOME];
    char telefone[TAM_TEL];
} Registro;

/* Nomes dos arquivos passados por linha de comando (globais por conveniência) */
static const char *arquivoRegistros;   /* ex: "registros.dat"  */
static const char *arquivoArvore;      /* ex: "arvore.txt"     */

/* ------------------------------------------------------------------ */
/*  carregarArvore                                                      */
/*                                                                      */
/*  Lê o arquivo binário sequencialmente UMA ÚNICA VEZ na iniciali-    */
/*  zação e reconstrói a árvore B em memória.                          */
/*  Cada chave guarda o índice (pos) do seu registro → acesso direto.  */
/* ------------------------------------------------------------------ */

ArvoreB carregarArvore() {
    ArvoreB arv = criarArvB();

    FILE *fp = fopen(arquivoRegistros, "rb");
    if (!fp) {
        printf("Arquivo '%s' nao encontrado. Iniciando com base vazia.\n\n",
               arquivoRegistros);
        return arv;
    }

    Registro reg;
    int pos = 0;   /* índice do registro (0-based) */

    while (fread(&reg, sizeof(Registro), 1, fp) == 1) {
        arv = inserirComPos(arv, reg.matricula, pos);
        pos++;
    }

    fclose(fp);
    printf("%d registro(s) carregado(s) do arquivo '%s'.\n\n",
           pos, arquivoRegistros);
    return arv;
}

/* ------------------------------------------------------------------ */
/*  cadastrar                                                           */
/*                                                                      */
/*  1. Lê matrícula, nome e telefone.                                   */
/*  2. Verifica duplicidade na árvore B (sem tocar no arquivo).        */
/*  3. Anexa o registro ao final do arquivo binário.                   */
/*  4. Insere a chave (matrícula + índice do registro) na árvore B.   */
/* ------------------------------------------------------------------ */

void cadastrar(ArvoreB *arv) {
    Registro reg;
    Pagina  *pagEncontrada;
    int      posEncontrada;

    printf("\n--- CADASTRAR ---\n");

    printf("Matricula : ");
    scanf("%d", &reg.matricula);
    getchar();  /* descarta o '\n' */

    /* Verifica duplicidade antes de qualquer I/O em arquivo */
    if (buscaArvB(*arv, reg.matricula, &pagEncontrada, &posEncontrada)) {
        printf("ERRO: matricula %d ja cadastrada.\n", reg.matricula);
        return;
    }

    printf("Nome      : ");
    fgets(reg.nome, TAM_NOME, stdin);
    reg.nome[strcspn(reg.nome, "\n")] = '\0';

    printf("Telefone  : ");
    fgets(reg.telefone, TAM_TEL, stdin);
    reg.telefone[strcspn(reg.telefone, "\n")] = '\0';

    /* Abre em modo append binário (cria o arquivo se não existir) */
    FILE *fp = fopen(arquivoRegistros, "ab");
    if (!fp) {
        printf("ERRO: nao foi possivel abrir o arquivo para escrita.\n");
        return;
    }

    /*
     * Calcula o índice que este registro terá:
     *   tamanho atual do arquivo / tamanho de um registro
     */
    fseek(fp, 0, SEEK_END);
    int recPos = (int)(ftell(fp) / sizeof(Registro));

    fwrite(&reg, sizeof(Registro), 1, fp);
    fclose(fp);

    /* Insere na árvore B com o índice real do registro */
    *arv = inserirComPos(*arv, reg.matricula, recPos);

    printf("Cadastro realizado com sucesso! (indice no arquivo: %d)\n", recPos);
}

/* ------------------------------------------------------------------ */
/*  pesquisar                                                           */
/*                                                                      */
/*  1. Busca a matrícula na árvore B.                                  */
/*  2. Obtém o índice (pos) armazenado na chave encontrada.            */
/*  3. Usa fseek para ir DIRETAMENTE ao registro no arquivo.           */
/*     → nenhuma varredura sequencial do arquivo é feita.              */
/* ------------------------------------------------------------------ */

void pesquisar(ArvoreB arv) {
    int     matricula;
    Pagina *pagEncontrada;
    int     posChave;

    printf("\n--- PESQUISAR ---\n");
    printf("Matricula : ");
    scanf("%d", &matricula);
    getchar();

    if (!buscaArvB(arv, matricula, &pagEncontrada, &posChave)) {
        printf("Matricula %d nao encontrada.\n", matricula);
        return;
    }

    /* Índice do registro no arquivo (acesso direto) */
    int recPos = pagEncontrada->chaves[posChave].pos;

    FILE *fp = fopen(arquivoRegistros, "rb");
    if (!fp) {
        printf("ERRO: nao foi possivel abrir o arquivo para leitura.\n");
        return;
    }

    /* Posiciona diretamente no registro sem varredura */
    fseek(fp, (long)recPos * sizeof(Registro), SEEK_SET);

    Registro reg;
    if (fread(&reg, sizeof(Registro), 1, fp) != 1) {
        printf("ERRO: falha ao ler o registro no arquivo.\n");
        fclose(fp);
        return;
    }
    fclose(fp);

    printf("\nRegistro encontrado:\n");
    printf("  Matricula : %d\n", reg.matricula);
    printf("  Nome      : %s\n", reg.nome);
    printf("  Telefone  : %s\n", reg.telefone);
}

/* ------------------------------------------------------------------ */
/*  gravar                                                              */
/*                                                                      */
/*  Grava um arquivo TEXTO com o conteúdo de todos os nós da árvore B. */
/*                                                                      */
/*  Formato do arquivo:                                                 */
/*    Linha 1   → endereço (ponteiro) do nó raiz                       */
/*    Próximas  → um nó por bloco, em percurso pré-ordem:              */
/*                  Endereco: <ptr>                                     */
/*                  Qtd: <n>                                            */
/*                  Chave[i]: valor=<v>  pos=<p>  (para cada chave)    */
/*                  Filho[i]: <ptr>               (para cada filho)    */
/*                  ---                           (separador de nós)   */
/*                                                                      */
/*  O percurso pré-ordem garante que cada pai aparece antes dos filhos */
/*  no arquivo, tornando a estrutura da árvore legível de cima a baixo.*/
/* ------------------------------------------------------------------ */

/* Função auxiliar: percorre em pré-ordem e escreve cada nó no arquivo */
static void gravarNos(FILE *fp, Pagina *pag) {
    if (!pag) return;

    fprintf(fp, "Endereco: %p\n", (void *)pag);
    fprintf(fp, "Qtd: %d\n", pag->qtd);

    for (int i = 0; i < pag->qtd; i++)
        fprintf(fp, "  Chave[%d]: valor=%d  pos=%d\n",
                i, pag->chaves[i].valor, pag->chaves[i].pos);

    for (int i = 0; i <= pag->qtd; i++)
        fprintf(fp, "  Filho[%d]: %p\n", i, (void *)pag->filho[i]);

    fprintf(fp, "---\n");

    /* Desce recursivamente para cada filho */
    for (int i = 0; i <= pag->qtd; i++)
        gravarNos(fp, pag->filho[i]);
}

void gravar(ArvoreB arv) {
    printf("\n--- GRAVAR ---\n");

    FILE *fp = fopen(arquivoArvore, "w");
    if (!fp) {
        printf("ERRO: nao foi possivel criar o arquivo '%s'.\n", arquivoArvore);
        return;
    }

    /* Primeira linha: endereço do nó raiz */
    fprintf(fp, "Raiz: %p\n\n", (void *)arv);

    /* Demais linhas: informações de cada nó em pré-ordem */
    gravarNos(fp, arv);

    fclose(fp);
    printf("Arvore gravada com sucesso em '%s'.\n", arquivoArvore);
}

/* ------------------------------------------------------------------ */
/*  liberarArvore                                                       */
/*                                                                      */
/*  Percorre a árvore em pós-ordem e libera cada página com free().    */
/*  Pós-ordem garante que os filhos são liberados antes do pai.        */
/* ------------------------------------------------------------------ */

void liberarArvore(Pagina *pag) {
    if (!pag) return;

    /* Libera todos os filhos primeiro (pós-ordem) */
    for (int i = 0; i <= pag->qtd; i++)
        liberarArvore(pag->filho[i]);

    free(pag);
}

/* ------------------------------------------------------------------ */
/*  sair                                                                */
/*                                                                      */
/*  Libera toda a memória da árvore B e encerra o programa.            */
/* ------------------------------------------------------------------ */

void sair(ArvoreB *arv) {
    liberarArvore(*arv);
    *arv = NULL;
    printf("Memoria liberada. Encerrando o programa.\n");
}

/* ------------------------------------------------------------------ */
/*  Menu principal                                                      */
/*                                                                      */
/*  Uso:     ./programa <arquivo_registros> <arquivo_arvore>           */
/*  Exemplo: ./programa registros.dat arvore.txt                       */
/* ------------------------------------------------------------------ */

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Uso: %s <arquivo_registros> <arquivo_arvore>\n", argv[0]);
        printf("Exemplo: %s registros.dat arvore.txt\n", argv[0]);
        return 1;
    }

    arquivoRegistros = argv[1];
    arquivoArvore    = argv[2];

    ArvoreB arv = carregarArvore();
    int opcao;

    do {
        printf("=============================\n");
        printf("         MENU PRINCIPAL      \n");
        printf("=============================\n");
        printf("  1. Cadastrar\n");
        printf("  2. Pesquisar\n");
        printf("  3. Gravar\n");
        printf("  4. Sair\n");
        printf("-----------------------------\n");
        printf("Opcao: ");
        scanf("%d", &opcao);
        getchar();

        switch (opcao) {
            case 1: cadastrar(&arv);  break;
            case 2: pesquisar(arv);   break;
            case 3: gravar(arv);      break;
            case 4: sair(&arv);       break;
            default: printf("Opcao invalida. Tente novamente.\n");
        }
        printf("\n");

    } while (opcao != 4);

    return 0;
}