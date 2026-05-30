#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grafo.h"

/* ─── ler_pgm ──────────────────────────────────────────────────
 * Lê arquivo .pgm P2 (texto puro).
 * Retorna Imagem* ou NULL em erro. Liberar com liberar_imagem().
 * ─────────────────────────────────────────────────────────────── */
Imagem *ler_pgm(const char *arquivo) {
	FILE *f = fopen(arquivo, "r");
	if (!f) {
		fprintf(stderr, "[ERRO] ler_pgm: não abriu '%s'\n", arquivo);
		return NULL; 
	}

	//lê magic number
	char magic[4];
	if (fscanf(f, "%3s", magic) != 1 || strcmp(magic, "P2") != 0) {
		fprintf(stderr, "[ERRO] ler_pgm: arquivo não é P2 (era '%s')\n", magic);
		fclose(f);
		return NULL; 
	}

	//consome restante da linha do magic
	char buf[512];
	if (fgets(buf, sizeof(buf), f) == NULL) {
		fprintf(stderr, "[ERRO] ler_pgm: EOF apos magic\n"); fclose(f); return NULL;
	}

	//pula linhas de comentário e busca dimensões
	int largura = 0, altura = 0;
	while (1) {
		if (fgets(buf, sizeof(buf), f) == NULL) {
			fprintf(stderr, "[ERRO] ler_pgm: EOF ao ler dimensoes\n"); fclose(f); return NULL;
		}
		if (buf[0] == '#') continue;
		if (sscanf(buf, "%d %d", &largura, &altura) == 2) break;
		// caso a linha contenha apenas um número, tentar ler a próxima
	}

	//lê valor máximo
	int max_val = 0;
	if (fscanf(f, "%d", &max_val) != 1) {
		fprintf(stderr, "[ERRO] ler_pgm: falha ao ler max_val\n"); fclose(f); return NULL;
	}

	//aloca imagem
	Imagem *img = (Imagem *)malloc(sizeof(Imagem)); 
	if (!img) { fprintf(stderr, "[ERRO] ler_pgm: sem memoria\n"); fclose(f); return NULL; }
	img->largura = largura; 
	img->altura = altura; 	

	// Aloca TODOS os pixels num único bloco contíguo (1D)
	img->pixels = (int *)malloc(altura * largura * sizeof(int)); 
	if (!img->pixels) { free(img); fclose(f); fprintf(stderr, "[ERRO] ler_pgm: sem memoria\n"); return NULL; }
	
	// Leitura dos dados mapeados linearmente
	for (int y = 0; y < altura; y++) {
		for (int x = 0; x < largura; x++) {
			// Cálculo do índice linear: y * largura + x
			if (fscanf(f, "%d", &img->pixels[y * largura + x]) != 1) {
				free(img->pixels); free(img); fclose(f);
				fprintf(stderr, "[ERRO] dados de pixel incompletos\n"); return NULL;
			}
		}
	}

	fclose(f);
	printf("[OK] ler_pgm: '%s' %dx%d pixels\n", arquivo, largura, altura);
	return img;
}

/* ─── salvar_pgm ────────────────────────────────────────────────
 * Salva Imagem em disco como P2.
 * ─────────────────────────────────────────────────────────────── */
void salvar_pgm(const char *arquivo, const Imagem *img) {
	FILE *f = fopen(arquivo, "w");
	if (!f) {
		fprintf(stderr, "[ERRO] salvar_pgm: não abriu '%s'\n", arquivo);
		return;
	}

	fprintf(f, "P2\n%d %d\n255\n", img->largura, img->altura);
	for (int y = 0; y < img->altura; y++) {
		for (int x = 0; x < img->largura; x++)
			fprintf(f, "%d ", img->pixels[y * img->largura + x]); // Acesso linear
		fprintf(f, "\n");
	}
	fclose(f);
}

//liberar (memória da) imagem 
void liberar_imagem(Imagem *img) {
	if (!img) return; 
	free(img->pixels); // Libera o bloco único de pixels
	free(img);
}