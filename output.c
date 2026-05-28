#include <stdio.h>
#include <stdlib.h>
#include "grafo.h"

/* ─── salvar_caminho_ppm ────────────────────────────────────────
 * Recebe a imagem original e o Caminho* do Dijkstra (Int. 3).
 * Gera arquivo .ppm com o contorno pintado em vermelho.
 *
 * Como usar no main.c:
 *   Caminho c = dijkstra(g, origem, destino);
 *   salvar_caminho_ppm("saida.ppm", img, &c);
 * ─────────────────────────────────────────────────────────────── */
void salvar_caminho_ppm(const char *arquivo, 
			const Imagem *img, 
			const Caminho *c) {
	FILE *f = fopen(arquivo, "w");
	if (!f) {
		fprintf(stderr, "[ERRO] salvar_caminho_ppm: falha ao criar\n");
        	return; 
	}

	int N = img->largura * img->altura; 

	//mapa booleano: 1 = pixel faz parte do contorno 
	char *marca = (char *)calloc(N, sizeof(char));
	for (int i=0; i < c->tamanho; i++)
		marca[c->caminho[i]] = 1; 

		//cabeçalho PPM
		fprintf(f, "P3\n%d %d\n255\n", img->largura, img->altura);

		for (int y=0; y < img->altura; y++) {
			for (int x=0; x < img->largura; x++) {
				int id = y * img->largura + x; 
				int cinza = img->pixels[y][x]; 

				if (marca[id])
					fprintf(f, "255   0   0  "); //vermelho
				else 
					fprintf(f, "%3d %3d %3d  ", cinza, cinza, cinza);
			}
			fprintf(f, "\n");
		}

		free(marca); 
		fclose(f);
		printf("[OK] salvar_caminho_ppm: '%s' (%d pixels no contorno)\n", arquivo, c->tamanho);
}
