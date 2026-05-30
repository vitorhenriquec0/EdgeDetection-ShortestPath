#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "grafo.h"
#include "dijkstra.h"

typedef struct {
    int *pixels;
    int tamanho;
    int toca_borda;
    int objeto_claro;
} Segmento;

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

static unsigned char *construir_mascara_contorno(const Imagem *img, const Segmento *objeto, int threshold)
{
    int total = img->largura * img->altura;
    unsigned char *mascara = calloc((size_t)total, sizeof(unsigned char));
    if (!mascara) {
        return NULL;
    }

    for (int i = 0; i < objeto->tamanho; i++) {
        int id = objeto->pixels[i];
        int x = id % img->largura;
        int y = id / img->largura;
        if (borda_de_pixel(img, x, y, threshold, objeto->objeto_claro)) {
            mascara[id] = 1;
        }
    }

    return mascara;
}

static double centroide_x = 0.0;
static double centroide_y = 0.0;
static int ordem_largura = 0;

static int contar_marcados(const unsigned char *mascara, int total);

static int comparar_angulo_contorno(const void *a, const void *b)
{
    int id_a = *(const int *)a;
    int id_b = *(const int *)b;
    double ax = (double)(id_a % ordem_largura) - centroide_x;
    double ay = (double)(id_a / ordem_largura) - centroide_y;
    double bx = (double)(id_b % ordem_largura) - centroide_x;
    double by = (double)(id_b / ordem_largura) - centroide_y;

    double ang_a = atan2(ay, ax);
    double ang_b = atan2(by, bx);
    if (ang_a < ang_b) return -1;
    if (ang_a > ang_b) return 1;

    double dist_a = ax * ax + ay * ay;
    double dist_b = bx * bx + by * by;
    if (dist_a < dist_b) return -1;
    if (dist_a > dist_b) return 1;

    return (id_a < id_b) ? -1 : (id_a > id_b);
}

static int construir_ordem_contorno(const unsigned char *mascara, int largura, int altura, int **ordem_out, int *tamanho_out)
{
    int total = largura * altura;
    int contagem = contar_marcados(mascara, total);
    if (contagem < 2) {
        return 0;
    }

    int *ordem = malloc((size_t)contagem * sizeof(int));
    if (!ordem) {
        return 0;
    }

    double soma_x = 0.0;
    double soma_y = 0.0;
    int indice = 0;
    for (int i = 0; i < total; i++) {
        if (!mascara[i]) {
            continue;
        }
        ordem[indice++] = i;
        soma_x += (double)(i % largura);
        soma_y += (double)(i / largura);
    }

    centroide_x = soma_x / contagem;
    centroide_y = soma_y / contagem;
    ordem_largura = largura;
    qsort(ordem, (size_t)contagem, sizeof(int), comparar_angulo_contorno);

    *ordem_out = ordem;
    *tamanho_out = contagem;
    (void)altura;
    return 1;
}

static int contar_marcados(const unsigned char *mascara, int total)
{
    int contagem = 0;
    for (int i = 0; i < total; i++) {
        if (mascara[i]) {
            contagem++;
        }
    }
    return contagem;
}

static Caminho rastrear_contorno(const Imagem *img)
{
    Caminho c = { NULL, 0 };
    int threshold = otsu_threshold(img);

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

    unsigned char *mascara_contorno = construir_mascara_contorno(img, escolhido, threshold);
    if (!mascara_contorno) {
        fprintf(stderr, "[ERRO] Sem memoria para a mascara do contorno.\n");
        liberar_segmento(&escuro);
        liberar_segmento(&claro);
        return c;
    }

    int *ordem_contorno = NULL;
    int tamanho_contorno = 0;
    if (!construir_ordem_contorno(mascara_contorno, img->largura, img->altura, &ordem_contorno, &tamanho_contorno)) {
        fprintf(stderr, "[ERRO] Nao foi possivel ordenar o contorno.\n");
        free(mascara_contorno);
        liberar_segmento(&escuro);
        liberar_segmento(&claro);
        return c;
    }

    int origem = ordem_contorno[0];
    int destino = ordem_contorno[1];

    Grafo *g = criar_grafo_contorno(img, ordem_contorno, tamanho_contorno, origem, destino);
    if (!g) {
        fprintf(stderr, "[ERRO] Falha ao criar grafo do contorno.\n");
        free(ordem_contorno);
        free(mascara_contorno);
        liberar_segmento(&escuro);
        liberar_segmento(&claro);
        return c;
    }

    c = dijkstra(g, origem, destino);

    printf("[OK] threshold automatico: %d | objeto %s\n", threshold, escolhido->objeto_claro ? "claro" : "escuro");
    printf("[OK] pixels de contorno: %d | origem: %d | destino: %d\n", tamanho_contorno, origem, destino);
    printf("[OK] caminho dijkstra: %d pixels\n", c.tamanho);

    liberar_grafo(g);
    free(ordem_contorno);
    free(mascara_contorno);
    liberar_segmento(&escuro);
    liberar_segmento(&claro);
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