#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include "grafo.h" 
#include "heap.h"  

#define SEM_PRED (-1)

/* Usa o tipo Caminho declarado em grafo.h */


static Caminho reconstruir_caminho(int *pred, int origem, int destino)
{
    Caminho c = { NULL, 0 };

    /* Conta os nós */
    int tamanho = 0;
    int atual = destino;
    while (atual != SEM_PRED) {
        tamanho++;
        if (atual == origem) break;
        atual = pred[atual];
    }

    c.caminho = malloc(tamanho * sizeof(int));
    if (!c.caminho) return c;

    /* Preenche de trás para frente */
    atual = destino;
    for (int i = tamanho - 1; i >= 0; i--) {
        c.caminho[i] = atual;
        atual = pred[atual];
    }

    c.tamanho = tamanho;
    return c;
}


/* ------------------------------------------------------------------
 * dijkstra
 * Função principal. Retorna o caminho mínimo entre origem e destino
 * no grafo de pixels, que corresponde ao contorno detectado.
 * ------------------------------------------------------------------ */
Caminho dijkstra(Grafo *grafo, int origem, int destino)
{
    int V = grafo->num_nos;
    Caminho vazio = { NULL, 0 };

    double *dist = malloc(V * sizeof(double));
    int    *pred = malloc(V * sizeof(int));
    if (!dist || !pred) { free(dist); free(pred); return vazio; }

    /* Inicialização */
    for (int i = 0; i < V; i++) {
        dist[i] = DBL_MAX;
        pred[i] = SEM_PRED;
    }
    dist[origem] = 0.0;

    MinHeap *heap = criar_heap(V);
    inserir_heap(heap, origem, 0.0);

    /* Loop principal */
    while (!heap_vazia(heap)) {
        ItemHeap item = remover_min(heap);
        int u = item.id_no;

        /* Parada antecipada: chegou ao destino */
        if (u == destino) break;

        /* Descarta entradas desatualizadas */
        if (item.prioridade > dist[u]) continue;

        /* Relaxamento dos vizinhos */
        Aresta *a = grafo->lista_adj[u];
        while (a != NULL) {
            double nova = dist[u] + a->peso;
            if (nova < dist[a->destino]) {
                dist[a->destino] = nova;
                pred[a->destino] = u;
                inserir_heap(heap, a->destino, nova);
            }
            a = a->prox;
        }
    }

    liberar_heap(heap);

    Caminho resultado = reconstruir_caminho(pred, origem, destino);

    free(dist);
    free(pred);
    return resultado;
}


/* ------------------------------------------------------------------
 * liberar_caminho
 * Chame após o Integrante 1 terminar de gerar a imagem de saída.
 * ------------------------------------------------------------------ */
void liberar_caminho(Caminho *c)
{
    free(c->caminho);
    c->caminho = NULL;
    c->tamanho = 0;
}


/* ------------------------------------------------------------------
 * imprimir_caminho
 * Exibe as coordenadas (linha, col) de cada pixel do caminho.
 * Útil para testar antes de ter a saída visual pronta.
 * ------------------------------------------------------------------ */
void imprimir_caminho(Caminho *c, int largura)
{
    printf("Caminho encontrado: %d pixels\n", c->tamanho);
    for (int i = 0; i < c->tamanho; i++) {
        int lin = c->caminho[i] / largura;
        int col = c->caminho[i] % largura;
        printf("  [%d] pixel (%d, %d)\n", i, lin, col);
    }
}