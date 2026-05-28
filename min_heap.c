#include "min_heap.h"

// Troca dois elementos e mantém pos[] consistente
static void swap(MinHeap *h, int a, int b) {
    HeapItem tmp  = h->data[a];
    h->data[a]    = h->data[b];
    h->data[b]    = tmp;

    // atualiza o mapa de posições dos dois nós trocados
    h->pos[h->data[a].no] = a;
    h->pos[h->data[b].no] = b;
}

/* Sobe o elemento na posição i até restaurar a propriedade
 * pai.dist <= filho.dist  (heapify-up).
 * Complexidade: O(log n) */
static void sobe(MinHeap *h, int i) {
    while (i > 0) {
        int pai = (i - 1) / 2;
        if (h->data[pai].dist <= h->data[i].dist)
            break;          /* propriedade restaurada */
        swap(h, pai, i);
        i = pai;
    }
}

/* Desce o elemento na posição i até restaurar a propriedade
 * (heapify-down). Escolhe sempre o filho de menor distância.
 * Complexidade: O(log n) */
static void desce(MinHeap *h, int i) {
    while (1) {
        int menor = i;
        int esq   = 2 * i + 1;
        int dir   = 2 * i + 2;

        if (esq < h->tamanho &&
            h->data[esq].dist < h->data[menor].dist)
            menor = esq;

        if (dir < h->tamanho &&
            h->data[dir].dist < h->data[menor].dist)
            menor = dir;

        if (menor == i)
            break;          /* propriedade restaurada */

        swap(h, i, menor);
        i = menor;
    }
}


/*
 * heap_init — prepara a heap para uso.
 * total_nos deve ser LARGURA * ALTURA da imagem.
 * Marca todos os nós como "fora da heap" (pos = -1).
 */
void heap_init(MinHeap *h, int total_nos) {
    h->tamanho = 0;
    for (int i = 0; i < total_nos; i++)
        h->pos[i] = -1;   /* -1 significa: nó não está na heap */
}

/*
 * heap_inserir — adiciona o nó 'no' com distância 'dist'.
 * O elemento é inserido no fim e sobe até a posição correta.
 * Complexidade: O(log n)
 */
void heap_inserir(MinHeap *h, int no, float dist) {
    int i = h->tamanho++;    /* ocupa a próxima posição livre */

    h->data[i].no   = no;
    h->data[i].dist = dist;
    h->pos[no]      = i;

    sobe(h, i);              /* restaura a propriedade da heap */
}

/*
 * heap_remover_min — remove e retorna o nó de menor distância.
 * O último elemento ocupa a raiz e desce até a posição correta.
 * Complexidade: O(log n)
 *
 * ATENÇÃO: só chame se !heap_vazia(h).
 */
HeapItem heap_remover_min(MinHeap *h) {
    HeapItem minimo = h->data[0];    /* guarda o mínimo para retornar */

    h->tamanho--;

    if (h->tamanho > 0) {
        /* move o último elemento para a raiz */
        h->data[0]          = h->data[h->tamanho];
        h->pos[h->data[0].no] = 0;

        desce(h, 0);         /* restaura a propriedade da heap */
    }

    h->pos[minimo.no] = -1;  /* marca o nó como removido */
    return minimo;
}

void heap_diminuir_chave(MinHeap *h, int no, float nova_dist) {
    int i = h->pos[no];

    if (i == -1)
        return;              /* nó não está na heap, nada a fazer */

    if (nova_dist >= h->data[i].dist)
        return;              /* nova distância não é menor, ignora */

    h->data[i].dist = nova_dist;
    sobe(h, i);              /* nó pode ter subido na heap */
}

int heap_vazia(const MinHeap *h) {
    return h->tamanho == 0;
}

int heap_contem(const MinHeap *h, int no) {
    return h->pos[no] != -1;
}
