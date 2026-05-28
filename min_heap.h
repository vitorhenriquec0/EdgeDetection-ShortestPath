#ifndef MIN_HEAP_H
#define MIN_HEAP_H

#define MAX_HEAP 270000

typedef struct {
    int   no;    /* índice do pixel no grafo (y*LARGURA + x) */
    float dist;  /* distância acumulada pelo Dijkstra        */
} HeapItem;

typedef struct {
    HeapItem data[MAX_HEAP]; /* array que armazena os itens          */
    int      pos[MAX_HEAP];  /* pos[no] = índice do nó no array data */
    int      tamanho;        /* número de elementos atualmente       */
} MinHeap;

/* Inicializa a heap. Chame antes de qualquer outra função.
 * total_nos = LARGURA * ALTURA da imagem */
void heap_init(MinHeap *h, int total_nos);

/* Insere um novo nó com a distância informada. O(log n) */
void heap_inserir(MinHeap *h, int no, float dist);

/* Remove e retorna o nó com menor distância. O(log n)
 * Pré-condição: h->tamanho > 0  (cheque com heap_vazia antes) */
HeapItem heap_remover_min(MinHeap *h);

/* Diminui a chave de um nó já presente na heap. O(log n)
 * Usado pelo Dijkstra ao relaxar uma aresta:
 *   if (nova_dist < dist[v]) heap_diminuir_chave(&h, v, nova_dist); */
void heap_diminuir_chave(MinHeap *h, int no, float nova_dist);

/* Retorna 1 se a heap estiver vazia, 0 caso contrário */
int heap_vazia(const MinHeap *h);

/* Retorna 1 se o nó ainda está na heap, 0 se já foi removido */
int heap_contem(const MinHeap *h, int no);

#endif
