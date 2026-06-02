#include "min_heap.h"
#include <stdlib.h>


// Troca dois elementos da heap
static void swap(MinHeap *h, int a, int b)
{
    HeapItem tmp = h->data[a];
    h->data[a] = h->data[b];
    h->data[b] = tmp;

    h->pos[h->data[a].no] = a;
    h->pos[h->data[b].no] = b;
}

// Heapify-up
static void sobe(MinHeap *h, int i)
{
    while (i > 0)
    {
        int pai = (i - 1) / 2;

        if (h->data[pai].dist <= h->data[i].dist)
            break;

        swap(h, pai, i);
        i = pai;
    }
}

// Heapify-down
static void desce(MinHeap *h, int i)
{
    while (1)
    {
        int menor = i;
        int esq = 2 * i + 1;
        int dir = 2 * i + 2;

        if (esq < h->tamanho && h->data[esq].dist < h->data[menor].dist)
            menor = esq;

        if (dir < h->tamanho && h->data[dir].dist < h->data[menor].dist)
            menor = dir;

        if (menor == i)
            break;

        swap(h, i, menor);
        i = menor;
    }
}

// Inicialização
void heap_init(MinHeap *h, int total_nos)
{
    h->tamanho = 0;

    for (int i = 0; i < total_nos; i++)
        h->pos[i] = -1;
}

// Inserção
void heap_inserir(MinHeap *h, int no, double dist)
{
    int i = h->tamanho++;

    h->data[i].no = no;
    h->data[i].dist = dist;
    h->pos[no] = i;

    sobe(h, i);
}

// Remoção do mínimo
HeapItem heap_remover_min(MinHeap *h)
{
    HeapItem minimo = h->data[0];

    h->tamanho--;

    if (h->tamanho > 0)
    {
        h->data[0] = h->data[h->tamanho];
        h->pos[h->data[0].no] = 0;

        desce(h, 0);
    }

    h->pos[minimo.no] = -1;

    return minimo;
}

// Decrease-key
void heap_diminuir_chave(MinHeap *h, int no, double nova_dist)
{
    int i = h->pos[no];

    if (i == -1)
        return;

    if (nova_dist >= h->data[i].dist)
        return;

    h->data[i].dist = nova_dist;

    sobe(h, i);
}

// Verificações
int heap_vazia(const MinHeap *h)
{
    return h->tamanho == 0;
}

int heap_contem(const MinHeap *h, int no)
{
    return h->pos[no] != -1;
}

// Criação
MinHeap *criar_heap(int total_nos)
{
    MinHeap *h = malloc(sizeof(MinHeap));

    if (!h)
        return NULL;

    h->data = malloc(total_nos * sizeof(HeapItem));
    h->pos = malloc(total_nos * sizeof(int));

    if (!h->data || !h->pos)
    {
        free(h->data);
        free(h->pos);
        free(h);
        return NULL;
    }

    h->capacidade = total_nos;

    heap_init(h, total_nos);

    return h;
}

// Inserção ou diminuição de chave
void inserir_heap(MinHeap *h, int no, double dist)
{
    if (heap_contem(h, no))
        heap_diminuir_chave(h, no, dist);
    else
        heap_inserir(h, no, dist);
}

ItemHeap remover_min(MinHeap *h)
{
    return heap_remover_min(h);
}

// Liberação
void liberar_heap(MinHeap *h)
{
    if (!h)
        return;

    free(h->data);
    free(h->pos);
    free(h);
}