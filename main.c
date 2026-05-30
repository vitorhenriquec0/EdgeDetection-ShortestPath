#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "grafo.h"
#include "dijkstra.h"

/* * A sua implementação perfeita do limiar de Otsu!
 * Serve para descobrir dinamicamente onde o fundo acaba e o objeto começa.
 */
static int otsu_threshold(const Imagem *img)
{
    int hist[256] = {0};
    int total = img->largura * img->altura;
    long soma_total = 0;

    for (int i = 0; i < total; i++)
    {
        int v = img->pixels[i];
        if (v < 0)
            v = 0;
        else if (v > 255)
            v = 255;
        hist[v]++;
        soma_total += v;
    }

    long soma_fundo = 0;
    int peso_fundo = 0;
    double melhor_variancia = -1.0;
    int melhor_t = 127;

    for (int t = 0; t < 256; t++)
    {
        peso_fundo += hist[t];
        if (peso_fundo == 0)
            continue;

        int peso_objeto = total - peso_fundo;
        if (peso_objeto == 0)
            break;

        soma_fundo += (long)t * hist[t];
        double media_fundo = (double)soma_fundo / peso_fundo;
        double media_objeto = (double)(soma_total - soma_fundo) / peso_objeto;
        double diferenca = media_fundo - media_objeto;
        double variancia = (double)peso_fundo * (double)peso_objeto * diferenca * diferenca;

        if (variancia > melhor_variancia)
        {
            melhor_variancia = variancia;
            melhor_t = t;
        }
    }
    return melhor_t;
}

int main(int argc, char **argv)
{
    const char *arquivo_entrada = (argc > 1) ? argv[1] : "image.pgm";
    Imagem *img = ler_pgm(arquivo_entrada);
    if (!img)
        return 1;

    int W = img->largura;
    int H = img->altura;

    // 1. Usamos a magia do Otsu para descobrir o limiar da imagem (sem chutar valores)
    int threshold = otsu_threshold(img);
    printf("[OK] Threshold de Otsu calculado: %d\n", threshold);

    // 2. Acha o TOPO geométrico exato procurando a transição de Otsu
    int topo_y = -1, topo_x = -1;
    for (int y = 1; y < H - 1; y++)
    {
        for (int x = 1; x < W - 1; x++)
        {
            int pixel_atual = img->pixels[y * W + x];
            int pixel_abaixo = img->pixels[(y + 1) * W + x];

            // Se cruzou o limiar de Otsu (fundo -> objeto ou objeto -> fundo), achou a borda superior!
            if ((pixel_atual <= threshold && pixel_abaixo > threshold) ||
                (pixel_atual > threshold && pixel_abaixo <= threshold))
            {
                topo_y = y;
                topo_x = x;
                break;
            }
        }
        if (topo_y != -1)
            break;
    }

    if (topo_y == -1 || topo_x <= 0)
    {
        fprintf(stderr, "[ERRO] Não foi possível encontrar a borda do objeto.\n");
        liberar_imagem(img);
        return 1;
    }

    // 3. Constrói o GRAFO DA IMAGEM INTEIRA (Torna as EDs Essenciais)
    Grafo *g = criar_grafo(img);

    // 4. Bloqueio Topológico: A Cortina de Ferro
    // Descemos o muro desde o topo da tela até a penúltima linha (H - 2)
    // Isso força o algoritmo a contornar TUDO e atravessar apenas pelo chão.
    for (int y = 0; y < H - 1; y++)
    {
        int id_esq = y * W + (topo_x - 1);
        int id_dir = y * W + topo_x;

        // Bloqueia qualquer travessia da esquerda pra direita
        Aresta *a = g->lista_adj[id_esq];
        while (a)
        {
            if (a->destino % W >= topo_x)
                a->peso = 1e12; // Muro ativado
            a = a->prox;
        }

        // Bloqueia qualquer travessia da direita pra esquerda
        a = g->lista_adj[id_dir];
        while (a)
        {
            if (a->destino % W <= topo_x - 1)
                a->peso = 1e12; // Muro ativado
            a = a->prox;
        }
    }

    // 5. Posiciona Origem e Destino lado a lado
    int origem = topo_y * W + (topo_x - 1);
    int destino = topo_y * W + topo_x;

    printf("Topo detectado via Otsu: (%d, %d)\n", topo_x, topo_y);
    printf("Origem  (Esq do muro): %d\n", origem);
    printf("Destino (Dir do muro): %d\n", destino);

    // 6. Roda o DIJKSTRA que agora é obrigado a descobrir todo o caminho
    Caminho c = dijkstra(g, origem, destino);

    if (c.tamanho > 0)
    {
        printf("[OK] Contorno fechado pelo Dijkstra: %d pixels\n", c.tamanho);
        mostrar_caminho_ascii(img, &c);
        salvar_caminho_ascii("saida_ascii.txt", img, &c);
        salvar_caminho_ppm("saida.ppm", img, &c);
    }
    else
    {
        printf("[AVISO] Caminho não encontrado\n");
    }

    liberar_caminho(&c);
    liberar_grafo(g);
    liberar_imagem(img);
    return 0;
}