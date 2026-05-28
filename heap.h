#ifndef HEAP_H
#define HEAP_H

#include "min_heap.h"

typedef HeapItem ItemHeap;

#define id_no no
#define prioridade dist

//aloca e inicializa MinHeap 
static inline MinHeap *criar_heap(int total_nos) {
	MinHeap *h = (MinHeap *)malloc(sizeof(MinHeap));
	if (!h) return NULL;
	heap_init(h, total_nos); 
	return h;
}

//insere nó com prioridade (double -> float)
static inline void inserir_heap(MinHeap *h, int no, double dist) {
	if (heap_contem(h, no))
		heap_diminuir_chave(h, no, (float)dist); 
	else
		heap_inserir(h, no, (float)dist);
}

//remove e retorna o elemento de menor dist
static inline ItemHeap remover_min(MinHeap *h) {
	return heap_remover_min(h); 
}

//libera memória alocada por criar_heap 
static inline void liberar_heap(MinHeap *h) {
	free(h);
}

#endif
