#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "grafo.h"

Caminho dijkstra(Grafo *grafo, int origem, int destino);
void    liberar_caminho(Caminho *c);
void    imprimir_caminho(Caminho *c, int largura);

#endif