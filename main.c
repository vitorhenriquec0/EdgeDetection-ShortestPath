#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grafo.h"
#include "dijkstra.h"

typedef struct {
    int *pixels;
    int tamanho;
    int toca_borda;
    int objeto_claro;
} Segmento;

static int adicionar_pixel(Caminho *c, int id)
{
    int nova_capacidade = (c->tamanho == 0) ? 256 : c->tamanho * 2;
    if (c->tamanho + 1 > nova_capacidade) {
        nova_capacidade = c->tamanho + 1;
    }

    int *novo = realloc(c->caminho, (size_t)nova_capacidade * sizeof(int));
    if (!novo) {
        return 0;
    }

    c->caminho = novo;
    c->caminho[c->tamanho++] = id;
    return 1;
}

static void liberar_segmento(Segmento *s)
{
    free(s->pixels);
    s->pixels = NULL;
    s->tamanho = 0;
    s->toca_borda = 0;
    s->objeto_claro = 0;
}

static int pixel_objeto(int valor, int threshold, int objeto_claro)
{
    return objeto_claro ? (valor > threshold) : (valor <= threshold);
}

static int borda_de_pixel(const Imagem *img, int x, int y, int threshold, int objeto_claro)
{
    if (x < 0 || x >= img->largura || y < 0 || y >= img->altura) {
        return 0;
    }

    if (!pixel_objeto(img->pixels[y * img->largura + x], threshold, objeto_claro)) {
        return 0;
    }

    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) {
                continue;
            }

            int nx = x + dx;
            int ny = y + dy;
            if (nx < 0 || nx >= img->largura || ny < 0 || ny >= img->altura) {
                return 1;
            }

            if (!pixel_objeto(img->pixels[ny * img->largura + nx], threshold, objeto_claro)) {
                return 1;
            }
        }
    }

    return 0;
}

static int otsu_threshold(const Imagem *img)
{
    int hist[256] = {0};
    int total = img->largura * img->altura;
    long soma_total = 0;

    for (int i = 0; i < total; i++) {
        int v = img->pixels[i];
        if (v < 0) {
            v = 0;
        } else if (v > 255) {
            v = 255;
        }
        hist[v]++;
        soma_total += v;
    }

    long soma_fundo = 0;
    int peso_fundo = 0;
    double melhor_variancia = -1.0;
    int melhor_t = 127;

    for (int t = 0; t < 256; t++) {
        peso_fundo += hist[t];
        if (peso_fundo == 0) {
            continue;
        }

        int peso_objeto = total - peso_fundo;
        if (peso_objeto == 0) {
            break;
        }

        soma_fundo += (long)t * hist[t];
        double media_fundo = (double)soma_fundo / peso_fundo;
        double media_objeto = (double)(soma_total - soma_fundo) / peso_objeto;
        double diferenca = media_fundo - media_objeto;
        double variancia = (double)peso_fundo * (double)peso_objeto * diferenca * diferenca;

        if (variancia > melhor_variancia) {
            melhor_variancia = variancia;
            melhor_t = t;
        }
    }

    return melhor_t;
}

static int salvar_segmento(Segmento *dest, const int *origem, int tamanho, int toca_borda, int objeto_claro)
{
    int *novo = realloc(dest->pixels, (size_t)tamanho * sizeof(int));
    if (!novo && tamanho > 0) {
        return 0;
    }

    dest->pixels = novo;
    if (tamanho > 0) {
        memcpy(dest->pixels, origem, (size_t)tamanho * sizeof(int));
    }
    dest->tamanho = tamanho;
    dest->toca_borda = toca_borda;
    dest->objeto_claro = objeto_claro;
    return 1;
}

static Segmento extrair_melhor_segmento(const Imagem *img, int threshold, int objeto_claro)
{
    Segmento melhor = { NULL, 0, 1, objeto_claro };
    int total = img->largura * img->altura;
    unsigned char *visitado = calloc((size_t)total, sizeof(unsigned char));
    int *fila = malloc((size_t)total * sizeof(int));
    int *componente = malloc((size_t)total * sizeof(int));

    if (!visitado || !fila || !componente) {
        fprintf(stderr, "[ERRO] Sem memoria para analisar a imagem.\n");
        free(visitado);
        free(fila);
        free(componente);
        return melhor;
    }

    for (int idx = 0; idx < total; idx++) {
        if (visitado[idx] || !pixel_objeto(img->pixels[idx], threshold, objeto_claro)) {
            continue;
        }

        int inicio = 0;
        int fim = 0;
        int tamanho = 0;
        int toca_borda = 0;

        visitado[idx] = 1;
        fila[fim++] = idx;

        while (inicio < fim) {
            int atual = fila[inicio++];
            componente[tamanho++] = atual;

            int x = atual % img->largura;
            int y = atual / img->largura;
            if (x == 0 || x == img->largura - 1 || y == 0 || y == img->altura - 1) {
                toca_borda = 1;
            }

            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    if (dx == 0 && dy == 0) {
                        continue;
                    }

                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx < 0 || nx >= img->largura || ny < 0 || ny >= img->altura) {
                        continue;
                    }

                    int vizinho = ny * img->largura + nx;
                    if (!visitado[vizinho] && pixel_objeto(img->pixels[vizinho], threshold, objeto_claro)) {
                        visitado[vizinho] = 1;
                        fila[fim++] = vizinho;
                    }
                }
            }
        }

        if (!toca_borda && tamanho > melhor.tamanho) {
            if (!salvar_segmento(&melhor, componente, tamanho, toca_borda, objeto_claro)) {
                fprintf(stderr, "[ERRO] Sem memoria para guardar o melhor segmento.\n");
                liberar_segmento(&melhor);
                break;
            }
        }
    }

    free(visitado);
    free(fila);
    free(componente);
    return melhor;
}

static Caminho rastrear_contorno(const Imagem *img)
{
    Caminho c = { NULL, 0 };
    int threshold = otsu_threshold(img);
    int total = img->largura * img->altura;

    Segmento escuro = extrair_melhor_segmento(img, threshold, 0);
    Segmento claro = extrair_melhor_segmento(img, threshold, 1);

    Segmento *escolhido = NULL;
    if (escuro.tamanho > 0 && claro.tamanho > 0) {
        escolhido = (escuro.tamanho >= claro.tamanho) ? &escuro : &claro;
    } else if (escuro.tamanho > 0) {
        escolhido = &escuro;
    } else if (claro.tamanho > 0) {
        escolhido = &claro;
    }

    if (!escolhido) {
        fprintf(stderr, "[ERRO] Nenhum contorno interno foi detectado.\n");
        liberar_segmento(&escuro);
        liberar_segmento(&claro);
        return c;
    }

    for (int i = 0; i < escolhido->tamanho; i++) {
        int id = escolhido->pixels[i];
        int x = id % img->largura;
        int y = id / img->largura;
        if (borda_de_pixel(img, x, y, threshold, escolhido->objeto_claro)) {
            if (!adicionar_pixel(&c, id)) {
                fprintf(stderr, "[ERRO] Sem memoria ao montar o contorno.\n");
                liberar_caminho(&c);
                break;
            }
        }
    }

    printf("[OK] threshold automatico: %d | objeto %s\n", threshold, escolhido->objeto_claro ? "claro" : "escuro");
    printf("[OK] componente selecionado: %d pixels\n", escolhido->tamanho);

    liberar_segmento(&escuro);
    liberar_segmento(&claro);
    (void)total;
    return c;
}

int main(int argc, char **argv)
{
    const char *arquivo_entrada = (argc > 1) ? argv[1] : "image.pgm";
    Imagem *img = ler_pgm(arquivo_entrada);
    if (!img) {
        return 1;
    }

    Caminho c = rastrear_contorno(img);

    if (c.tamanho > 0) {
        printf("[OK] contorno fechado: %d pixels\n", c.tamanho);
        mostrar_caminho_ascii(img, &c);
        salvar_caminho_ascii("saida_ascii.txt", img, &c);
        salvar_caminho_ppm("saida.ppm", img, &c);
    } else {
        printf("[AVISO] Nenhum contorno foi rastreado\n");
    }

    liberar_caminho(&c);
    liberar_imagem(img);
    return 0;
}