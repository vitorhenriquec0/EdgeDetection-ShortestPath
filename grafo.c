#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "grafo.h"

// Peso = 1 / (|I1 - I2| + 1)
// Borda → peso baixo → Dijkstra prefere
// Interior homogêneo → peso alto  → Dijkstra evita
static double calcular_peso_gradiente(const Imagem *img, int nx, int ny) 
{
	int W = img->largura;
	int H = img->altura;
	
	int v_centro = img->pixels[ny * W + nx];
	
	// Se o vizinho estiver fora da tela, finge que ele tem a mesma cor do centro
	int v_esq   = (nx > 0)     ? img->pixels[ny * W + (nx - 1)] : v_centro;
	int v_dir   = (nx < W - 1) ? img->pixels[ny * W + (nx + 1)] : v_centro;
	int v_cima  = (ny > 0)     ? img->pixels[(ny - 1) * W + nx] : v_centro;
	int v_baixo = (ny < H - 1) ? img->pixels[(ny + 1) * W + nx] : v_centro;

	int gx = v_dir - v_esq;
	int gy = v_baixo - v_cima;

	double mag = (double)(abs(gx) + abs(gy)); 
	return 1000000.0 / ((mag * mag) + 1.0);
}

// Adiciona aresta direcionada (origem -> destino) com peso dado
static void adicionar_aresta(Grafo *g, int origem, int destino, double peso)
{
	Aresta *a = (Aresta *)malloc(sizeof(Aresta));
	a->destino = destino;
	a->peso = peso;
	a->prox = g->lista_adj[origem];
	g->lista_adj[origem] = a;
}

// Transforma a Imagem em grafo de lista de adjacência
// Cada pixel conecta-se aos 8 vizinhos (conectividade 8)
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

	return g;
}

// Libera lista de adjacência e o próprio Grafo.
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

// Função para calcular o limiar de Otsu para segmentação automática da imagem
int otsu_threshold(const Imagem *img)
{
    int hist[256] = {0};
    int total = img->largura * img->altura;
    long soma_total = 0;

    for (int i = 0; i < total; i++)
    {
        int v = img->pixels[i];
        if (v < 0)
            v = 0;
        else if (v > 255)
            v = 255;
        hist[v]++;
        soma_total += v;
    }

    long soma_fundo = 0;
    int peso_fundo = 0;
    double melhor_variancia = -1.0;
    int melhor_t = 127;

    for (int t = 0; t < 256; t++)
    {
        peso_fundo += hist[t];
        if (peso_fundo == 0)
            continue;

        int peso_objeto = total - peso_fundo;
        if (peso_objeto == 0)
            break;

        soma_fundo += (long)t * hist[t];
        double media_fundo = (double)soma_fundo / peso_fundo;
        double media_objeto = (double)(soma_total - soma_fundo) / peso_objeto;
        double diferenca = media_fundo - media_objeto;
        double variancia = (double)peso_fundo * (double)peso_objeto * diferenca * diferenca;

        if (variancia > melhor_variancia)
        {
            melhor_variancia = variancia;
            melhor_t = t;
        }
    }
    return melhor_t;
}