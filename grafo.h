#ifndef GRAFO_H
#define GRAFO_H

/* ═══════════════════════════════════════════════════════
 * ARESTA — nó da lista de adjacência
 * peso: double para compatibilidade com dijkstra.c
 * ═══════════════════════════════════════════════════════ */
typedef struct Aresta {
	int destino; //id do pixel vizinho
	double peso; //1.0 / (|I1 - I2| + 1.0)
	struct Aresta *prox; //próxima aresta (lista ligada)  
} Aresta; 

/* ═══════════════════════════════════════════════════════
 * GRAFO — grafo de pixels em lista de adjacência
 * lista_adj: nome exigido pelo dijkstra.c do Int. 3
 * ═══════════════════════════════════════════════════════ */
typedef struct {
	int num_nos; //largura * altura
	int largura; 
	int altura; 
	Aresta **lista_adj; //lista_adj[id] -> lista do pixel 
} Grafo; 

/* ═══════════════════════════════════════════════════════
 * IMAGEM — matriz de pixels carregada do .pgm
 * ═══════════════════════════════════════════════════════ */
typedef struct {
	int *pixels;
	int largura; 
	int altura; 
} Imagem;

/* ═══════════════════════════════════════════════════════
 * CAMINHO — resultado do Dijkstra (usado por output.c)
 * Declarado aqui para evitar redefinição entre dijkstra.h
 * e dijkstra.c
 * ═══════════════════════════════════════════════════════ */
typedef struct {
	int *caminho; //array de IDs dos pixels de contorno 
	int tamanho; //número de pixels no caminho 
} Caminho; 


/* ═══════════════════════════════════════════════════════
 * PROTÓTIPOS — funções que você implementa
 * ═══════════════════════════════════════════════════════ */

//pgm_io.c 
Imagem *ler_pgm (const char *arquivo); 
void salvar_pgm (const char *arquivo, const Imagem *img);
void liberar_imagem (Imagem *img); 

//grafo.c 
Grafo *criar_grafo (const Imagem *img); 
void liberar_grafo (Grafo *g);

//output.c
void salvar_caminho_ppm(const char *arquivo, const Imagem *img, const Caminho *c); 
void salvar_caminho_ascii(const char *arquivo, const Imagem *img, const Caminho *c);
void mostrar_caminho_ascii(const Imagem *img, const Caminho *c);

#endif
