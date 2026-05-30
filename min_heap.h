#ifndef MIN_HEAP_H
#define MIN_HEAP_H


typedef struct {
    int   no;
    float dist;
} HeapItem;

typedef struct {
    HeapItem *data;
    int      *pos; 
    int      tamanho;  
    int      capacidade;
} MinHeap;

typedef HeapItem ItemHeap;

#define id_no no
#define prioridade dist

void heap_init(MinHeap *h, int total_nos);

void heap_inserir(MinHeap *h, int no, float dist);

HeapItem heap_remover_min(MinHeap *h);

void heap_diminuir_chave(MinHeap *h, int no, float nova_dist);

int heap_vazia(const MinHeap *h);

int heap_contem(const MinHeap *h, int no);

MinHeap *criar_heap(int total_nos);

void inserir_heap(MinHeap *h, int no, double dist);

ItemHeap remover_min(MinHeap *h);

void liberar_heap(MinHeap *h);

#endif
