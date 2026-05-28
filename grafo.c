#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "grafo.h"

/* ─── calcular_peso ─────────────────────────────────────────────
 * Fórmula: 1 / (|I1 - I2| + 1)
 * Borda (grande diferença) → peso baixo → Dijkstra prefere
 * Interior homogêneo       → peso alto  → Dijkstra evita
 * ─────────────────────────────────────────────────────────────── */
static double calcular_peso(int i1, int i2) {
	return 1.0 / (fabs((double)(i1-i2)) + 1.0);
}

/* ─── criar_grafo ───────────────────────────────────────────────
 * Transforma a Imagem em grafo de lista de adjacência.
 * Cada pixel conecta-se aos 8 vizinhos (conectividade 8).
 * ─────────────────────────────────────────────────────────────── */
Grafo *criar_grafo(const Imagem *img) {
	int W = img->largura; 
	int H = img->altura; 
	int N = W * H; 

	Grafo *g = (Grafo *)malloc(sizeof(Grafo)); 
	g->largura = W; 
	g->altura = H; 
	g->num_nos = N; 
	g->lista_adj = (Aresta **)calloc(N, sizeof(Aresta *)); 

	//deslocamentos para 8 vizinhos (dx, dy)
	int dx[] = {-1,  0,  1, -1, 1, -1, 0, 1};
	int dy[] = {-1, -1, -1,  0, 0,  1, 1, 1};

	for (int y=0; y < H; y++) {
		for (int x=0; x < W; x++) {
			int id_u = y * W + x; 
			
			for (int d=0; d < 8; d++) {
				int nx = x + dx[d]; 
				int ny = y + dy[d]; 

				//ignora vizinhos fora da imagem 
				if (nx < 0 || nx >= W || ny < 0 || ny >= H)
					continue; 
				
				int id_v = ny * W + nx; 
				double peso = calcular_peso(
					img->pixels[y][x]; 
					img->pixels[ny][nx]; 
					); 

				//insere aresta u->v na lista de u
				Aresta *a = (Aresta *)malloc(sizeof(Aresta)); 
				a->destino = id_v; 
				a->peso = peso; 
				a->prox = g->lista_adj[id_u];
				g->lista_adj[id_u] = a; //insere no inicio
			}
		}
	}

	printf("[OK] criar_grafo: %d nos, ~%d arestas\n", N, N*8);
	return g; 
}


/* ─── liberar_grafo ─────────────────────────────────────────────
 * Libera lista de adjacência e o próprio Grafo.
 * ─────────────────────────────────────────────────────────────── */
void liberar_grafo(Grafo *g) {
	if (!g) return; 
	for (int i=0; i < g->num_nos; i++) {
		Aresta *a = g->lista_adj[i]; 
		while(a) {
			Aresta *prox = a->prox; 
			free(a);
			a = prox; 
		}
	}
	free(g->lista_adj); 
	free(g);
}
