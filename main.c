#include <stdio.h>
#include <stdlib.h>
#include "grafo.h"
#include "dijkstra.h"

int main(void) {
	//lê imagem
	Imagem *img = ler_pgm("radiografia.pgm");
	if (!img) return 1;

	//constrói o grafo
	Grafo *g = criar_grafo(img);

	//define os pontos A e B
	//conversão (linha, coluna) -> id do pixel
	int origem = 100 * img->largura + 80; 
	int destino = 300 * img->largura + 200; 
	printf("Origem: (%d, %d) id=%d\n", 100, 80, origem);
	printf("Destino: (%d, %d), id=%d\n", 300, 200, destino);

	//roda o Dijkstra
	Caminho c = dijkstra(g, origem, destino); 

	//gera imagem de saída
	if (c.tamanho > 0) {
		salvar_caminho_ppm("saida.ppm", img, &c);
		imprimir_caminho(&c, img->largura);
	} else {
		printf("[AVISO] Caminho não encontrado");
	}

	//libera memória
	liberar_caminho(&c);
	liberar_grafo(g);
	liberar_imagem(img);

	return 0;
}
