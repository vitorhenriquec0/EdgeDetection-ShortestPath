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
	fgets(buf, sizeof(buf), f);

	//pula linhas de comentário, lê dimensões
	int largura, altura; 
	while (1) {
		fgets(buf, sizeof(buf), f); 
		if (buf[0] != '#') {
			sscanf(buf, "%d %d", &largura, &altura);
			break; 	
		}
	}

	//lê valor máximo 
	int max_val; 
	fscanf(f, "%d", &max_val); 

	//aloca imagem
	Imagem *img = (Imagem *)malloc(sizeof(Imagem)); 
	img->largura = largura; 
	img->altura = altura; 	

	//aloca matriz pixels[altura][largura] 
	img->pixels = (int **)malloc(altura * sizeof(int *)); 
	for (int y=0; y < altura; y++) {
		img->pixels[y] = (int *)malloc(largura * sizeof(int)); 
		for (int x=0; x < largura; x++)
			fscanf(f, "%d", &img->pixels[y][x]);
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
		fprintf(stderr, "[ERRO] salvar_pgm\n"); return; 
	}
	fprintf(f, "P2\n%d %d\n255\n", img->largura, img->altura);
	for (int y=0; y < img->altura; y++) {
		for (int x=0; x < img->largura; x++)
			fprintf(f, "%d", img->pixels[y][x]);
		fprintf(f, "\n");
	}
	fclose(f);
}

//liberar (memória da) imagem 
void liberar_imagem(Imagem *img) {
	if (!img) return; 
	for (int y=0; y < img->altura; y++) 
		free(img->pixels[y]);
	free(img->pixels);
	free(img);
}
