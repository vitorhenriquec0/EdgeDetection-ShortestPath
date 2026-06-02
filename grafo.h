#ifndef GRAFO_H
#define GRAFO_H

typedef struct Aresta {
	int destino; // id do pixel vizinho
	double peso;
	struct Aresta *prox; // próxima aresta
} Aresta; 

typedef struct {
	int num_nos; // largura * altura
	int largura; 
	int altura; 
	Aresta **lista_adj; // lista_adj[id] -> lista do pixel 
} Grafo; 

typedef struct {
	int *pixels;
	int largura; 
	int altura; 
} Imagem;

// Resultado do Dijkstra, declarado aqui para evitar redefinição entre dijkstra.h e dijkstra.c
typedef struct {
	int *caminho; //array de IDs dos pixels de contorno 
	int tamanho; //número de pixels no caminho 
} Caminho; 

Imagem *ler_pgm (const char *arquivo); 
void salvar_pgm (const char *arquivo, const Imagem *img);
void liberar_imagem (Imagem *img); 
Grafo *criar_grafo (const Imagem *img); 
void liberar_grafo (Grafo *g);
void salvar_caminho_ppm(const char *arquivo, const Imagem *img, const Caminho *c); 
void salvar_caminho_ascii(const char *arquivo, const Imagem *img, const Caminho *c);
void mostrar_caminho_ascii(const Imagem *img, const Caminho *c);
int otsu_threshold(const Imagem *img);

#endif
