#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grafo.h"

// Leitura de imagem PGM (P2) do disco
Imagem *ler_pgm(const char *arquivo) 
{
	FILE *f = fopen(arquivo, "r");
	if (!f) 
	{
		fprintf(stderr, "Erro - ler_pgm: não abriu '%s'\n", arquivo);
		return NULL; 
	}

	// Lê magic number
	char magic[4];
	if (fscanf(f, "%3s", magic) != 1 || strcmp(magic, "P2") != 0) 
	{
		fprintf(stderr, "Erro - ler_pgm: arquivo não é P2 (era '%s')\n", magic);
		fclose(f);
		return NULL; 
	}

	// Consome restante da linha do magic
	char buf[512];
	if (fgets(buf, sizeof(buf), f) == NULL) {
		fprintf(stderr, "Erro - ler_pgm: EOF apos magic\n");
		fclose(f);
		return NULL;
	}

	// Pula linhas de comentário e busca dimensões
	int largura = 0, altura = 0;
	while (1) {
		if (fgets(buf, sizeof(buf), f) == NULL) 
		{
			fprintf(stderr, "Erro - ler_pgm: EOF ao ler dimensoes\n");
			fclose(f);
			return NULL;
		}
		if (buf[0] == '#') continue;
		if (sscanf(buf, "%d %d", &largura, &altura) == 2) break;
		// caso a linha contenha apenas um número, tentar ler a próxima
	}

	// Lê valor máximo
	int max_val = 0;
	if (fscanf(f, "%d", &max_val) != 1) 
	{
		fprintf(stderr, "Erro - ler_pgm: falha ao ler max_val\n");
		fclose(f);
		return NULL;
	}

	// Aloca imagem
	Imagem *img = (Imagem *)malloc(sizeof(Imagem)); 
	if (!img) { fprintf(stderr, "Erro - ler_pgm: sem memoria\n"); fclose(f); return NULL; }
	img->largura = largura; 
	img->altura = altura; 	

	// Aloca TODOS os pixels num único bloco contíguo (1D)
	img->pixels = (int *)malloc(altura * largura * sizeof(int)); 
	if (!img->pixels) { free(img); fclose(f); fprintf(stderr, "Erro - ler_pgm: sem memoria\n"); return NULL; }
	
	// Leitura dos dados mapeados linearmente
	for (int y = 0; y < altura; y++) 
	{
		for (int x = 0; x < largura; x++) 
		{
			// Cálculo do índice linear: y * largura + x
			if (fscanf(f, "%d", &img->pixels[y * largura + x]) != 1) 
			{
				free(img->pixels); free(img); fclose(f);
				fprintf(stderr, "Erro: dados de pixel incompletos\n"); return NULL;
			}
		}
	}

	fclose(f);
	return img;
}

// Salvar imagem PGM (P2) no disco
void salvar_pgm(const char *arquivo, const Imagem *img) 
{
	FILE *f = fopen(arquivo, "w");
	if (!f) 
	{
		fprintf(stderr, "Errro - salvar_pgm: não abriu '%s'\n", arquivo);
		return;
	}

	fprintf(f, "P2\n%d %d\n255\n", img->largura, img->altura);
	for (int y = 0; y < img->altura; y++) 
	{
		for (int x = 0; x < img->largura; x++)
			fprintf(f, "%d ", img->pixels[y * img->largura + x]); // Acesso linear
		fprintf(f, "\n");
	}
	fclose(f);
}

// Liberar memória da imagem 
void liberar_imagem(Imagem *img) 
{
	if (!img) return; 
	free(img->pixels); // Libera o bloco único de pixels
	free(img);
}