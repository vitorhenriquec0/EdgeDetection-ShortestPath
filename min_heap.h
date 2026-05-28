#ifndef MIN_HEAP_H
#define MIN_HEAP_H

#define MAX_HEAP 270000

typedef struct {
    int   no;
    float dist;
} HeapItem;

typedef struct {
    HeapItem data[MAX_HEAP];
    int      pos[MAX_HEAP];
    int      tamanho;  
} MinHeap;

void heap_init(MinHeap *h, int total_nos);

void heap_inserir(MinHeap *h, int no, float dist);

HeapItem heap_remover_min(MinHeap *h);

void heap_diminuir_chave(MinHeap *h, int no, float nova_dist);

int heap_vazia(const MinHeap *h);

int heap_contem(const MinHeap *h, int no);

#endif
