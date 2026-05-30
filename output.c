#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grafo.h"

#define PREVIEW_LARGURA_MAX 80

static char intensidade_para_char(int cinza) {
	static const char ramp[] = " .:-=+*#%@";
	int nivel = (int)((cinza * (int)(sizeof(ramp) - 2)) / 255);
	if (nivel < 0) nivel = 0;
	if (nivel > (int)(sizeof(ramp) - 2)) nivel = (int)(sizeof(ramp) - 2);
	return ramp[nivel];
}

static char *montar_mapa_caminho(const Caminho *c, int total_pixels) {
	char *marca = (char *)calloc((size_t)total_pixels, sizeof(char));
	if (!marca) return NULL;
	if (!c || !c->caminho) return marca;
	for (int i = 0; i < c->tamanho; i++) {
		int id = c->caminho[i];
		if (id >= 0 && id < total_pixels) {
			marca[id] = 1;
		}
	}
	return marca;
}

static void renderizar_ascii(FILE *out, const Imagem *img, const Caminho *c, int usar_ansi) {
	int total_pixels = img->largura * img->altura;
	char *marca = montar_mapa_caminho(c, total_pixels);
	if (!marca) {
		fprintf(stderr, "[ERRO] ASCII: sem memoria\n");
		return;
	}

	int largura_saida = img->largura < PREVIEW_LARGURA_MAX ? img->largura : PREVIEW_LARGURA_MAX;
	if (largura_saida < 1) largura_saida = 1;
	int altura_saida = (img->altura * largura_saida) / img->largura / 2;
	if (altura_saida < 1) altura_saida = 1;

	fprintf(out, "ASCII preview %dx%d -> %dx%d\n", img->largura, img->altura, largura_saida, altura_saida);
	fprintf(out, "Legenda: '%c' claro ... '%c' escuro; '@' caminho\n", intensidade_para_char(0), intensidade_para_char(255));

	for (int sy = 0; sy < altura_saida; sy++) {
		int y0 = sy * img->altura / altura_saida;
		int y1 = (sy + 1) * img->altura / altura_saida;
		if (y1 <= y0) y1 = y0 + 1;
		if (y1 > img->altura) y1 = img->altura;

		for (int sx = 0; sx < largura_saida; sx++) {
			int x0 = sx * img->largura / largura_saida;
			int x1 = (sx + 1) * img->largura / largura_saida;
			if (x1 <= x0) x1 = x0 + 1;
			if (x1 > img->largura) x1 = img->largura;

			long soma = 0;
			int contagem = 0;
			int no_caminho = 0;

			for (int y = y0; y < y1; y++) {
				for (int x = x0; x < x1; x++) {
					int id = y * img->largura + x;
					soma += img->pixels[y * img->largura + x];
					contagem++;
					if (marca[id]) no_caminho = 1;
				}
			}

			int cinza = contagem > 0 ? (int)(soma / contagem) : 0;
			char ch = no_caminho ? '@' : intensidade_para_char(cinza);

			if (usar_ansi) {
				if (no_caminho) {
					fprintf(out, "\x1b[31;1m%c\x1b[0m", ch);
				} else {
					fprintf(out, "\x1b[38;2;%d;%d;%dm%c\x1b[0m", cinza, cinza, cinza, ch);
				}
			} else {
				fputc(ch, out);
			}
		}
		fputc('\n', out);
	}

	free(marca);
}

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

	// mapa booleano: 1 = pixel faz parte do contorno
	char *marca = (char *)calloc(N, sizeof(char));
	if (!marca) { fclose(f); fprintf(stderr, "[ERRO] salvar_caminho_ppm: sem memoria\n"); return; }
	if (c && c->caminho) {
		for (int i = 0; i < c->tamanho; i++) {
			int idx = c->caminho[i];
			if (idx >= 0 && idx < N) {
				marca[idx] = 1;
			}
		}
	}

	// cabeçalho PPM
	fprintf(f, "P3\n%d %d\n255\n", img->largura, img->altura);

		for (int y=0; y < img->altura; y++) {
			for (int x=0; x < img->largura; x++) {
				int id = y * img->largura + x; 
				int cinza = img->pixels[y * img->largura + x];

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

	void salvar_caminho_ascii(const char *arquivo, const Imagem *img, const Caminho *c) {
		FILE *f = fopen(arquivo, "w");
		if (!f) {
			fprintf(stderr, "[ERRO] salvar_caminho_ascii: falha ao criar\n");
			return;
		}

		fprintf(f, "# ASCII preview do caminho\n");
		fprintf(f, "# imagem: %dx%d\n", img->largura, img->altura);
		fprintf(f, "# caminho: %d pixels\n", c ? c->tamanho : 0);
		renderizar_ascii(f, img, c, 0);

		fclose(f);
		printf("[OK] salvar_caminho_ascii: '%s'\n", arquivo);
	}

	void mostrar_caminho_ascii(const Imagem *img, const Caminho *c) {
		renderizar_ascii(stdout, img, c, 1);
	}
