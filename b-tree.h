#define ORDER 4

typedef struct node {
    int qtd;
    char *keys[ORDER - 1];
    int pos[ORDER - 1];
    struct node *ptr[ORDER];
} node_t;
