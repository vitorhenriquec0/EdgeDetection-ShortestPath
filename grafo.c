#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "grafo.h"

/* ─── calcular_peso ─────────────────────────────────────────────
 * Fórmula: 1 / (|I1 - I2| + 1)
 * Borda (grande diferença) → peso baixo → Dijkstra prefere
 * Interior homogêneo       → peso alto  → Dijkstra evita
 * ─────────────────────────────────────────────────────────────── */
static double calcular_peso_gradiente(const Imagem *img, int nx, int ny) {
	int W = img->largura;
	int H = img->altura;
	
	// Pega o valor central
	int v_centro = img->pixels[ny * W + nx];
	
	// Se o vizinho estiver fora da tela, finge que ele tem a mesma cor do centro
	// Isso mata o "gradiente fantasma" nas bordas da imagem
	int v_esq   = (nx > 0)     ? img->pixels[ny * W + (nx - 1)] : v_centro;
	int v_dir   = (nx < W - 1) ? img->pixels[ny * W + (nx + 1)] : v_centro;
	int v_cima  = (ny > 0)     ? img->pixels[(ny - 1) * W + nx] : v_centro;
	int v_baixo = (ny < H - 1) ? img->pixels[(ny + 1) * W + nx] : v_centro;

	int gx = v_dir - v_esq;
	int gy = v_baixo - v_cima;

	double mag = (double)(abs(gx) + abs(gy)); 
	return 1000000.0 / ((mag * mag) + 1.0);
}

static void adicionar_aresta(Grafo *g, int origem, int destino, double peso)
{
	Aresta *a = (Aresta *)malloc(sizeof(Aresta));
	a->destino = destino;
	a->peso = peso;
	a->prox = g->lista_adj[origem];
	g->lista_adj[origem] = a;
}

static void adicionar_aresta_bidirecional(Grafo *g, int u, int v, double peso)
{
	adicionar_aresta(g, u, v, peso);
	adicionar_aresta(g, v, u, peso);
}

/* ─── criar_grafo ───────────────────────────────────────────────
 * Transforma a Imagem em grafo de lista de adjacência.
 * Cada pixel conecta-se aos 8 vizinhos (conectividade 8).
 * ─────────────────────────────────────────────────────────────── */
Grafo *criar_grafo(const Imagem *img)
{
	int W = img->largura;
	int H = img->altura;
	int N = W * H;

	Grafo *g = (Grafo *)malloc(sizeof(Grafo));
	g->largura = W;
	g->altura = H;
	g->num_nos = N;
	g->lista_adj = (Aresta **)calloc(N, sizeof(Aresta *));

	int dx[] = {-1, 0, 1, -1, 1, -1, 0, 1};
	int dy[] = {-1, -1, -1, 0, 0, 1, 1, 1};

	for (int y = 0; y < H; y++)
	{
		for (int x = 0; x < W; x++)
		{
			int id_u = y * W + x;

			for (int d = 0; d < 8; d++)
			{
				int nx = x + dx[d];
				int ny = y + dy[d];

				if (nx < 0 || nx >= W || ny < 0 || ny >= H)
					continue;

				int id_v = ny * W + nx;
				double peso = calcular_peso_gradiente(img, nx, ny);

				adicionar_aresta(g, id_u, id_v, peso);
			}
		}
	}

	printf("[OK] criar_grafo: %d nos, ~%d arestas\n", N, N * 8);
	return g;
}

/* ─── criar_grafo_contorno ──────────────────────────────────────
 * Cria um ciclo ordenado com os pixels do contorno.
 * A aresta entre origem e destino é bloqueada para forçar o Dijkstra
 * a percorrer o restante do ciclo.
 * ─────────────────────────────────────────────────────────────── */
Grafo *criar_grafo_contorno(const Imagem *img, const int *ordem_contorno, int tamanho_contorno, int origem, int destino)
{
	int N = img->largura * img->altura;

	Grafo *g = (Grafo *)malloc(sizeof(Grafo));
	g->largura = img->largura;
	g->altura = img->altura;
	g->num_nos = N;
	g->lista_adj = (Aresta **)calloc(N, sizeof(Aresta *));

	for (int i = 0; i < tamanho_contorno; i++)
	{
		int u = ordem_contorno[i];
		int v = ordem_contorno[(i + 1) % tamanho_contorno];
		double peso = ((u == origem && v == destino) || (u == destino && v == origem)) ? 1e12 : 1.0;

		adicionar_aresta_bidirecional(g, u, v, peso);
	}

	printf("[OK] criar_grafo_contorno: %d nos, %d pixels no contorno\n", N, tamanho_contorno);
	return g;
}

/* ─── liberar_grafo ─────────────────────────────────────────────
 * Libera lista de adjacência e o próprio Grafo.
 * ─────────────────────────────────────────────────────────────── */
void liberar_grafo(Grafo *g)
{
	if (!g)
		return;
	for (int i = 0; i < g->num_nos; i++)
	{
		Aresta *a = g->lista_adj[i];
		while (a)
		{
			Aresta *prox = a->prox;
			free(a);
			a = prox;
		}
	}
	free(g->lista_adj);
	free(g);
}