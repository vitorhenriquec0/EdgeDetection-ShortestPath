#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "grafo.h"
#include "dijkstra.h"

int main(int argc, char **argv)
{
    const char *arquivo_entrada = (argc > 1) ? argv[1] : "image.pgm";
    Imagem *img = ler_pgm(arquivo_entrada);
    if (!img)
        return 1;

    int W = img->largura;
    int H = img->altura;

    // 1. Usa Otsu para descobrir o limiar da imagem
    int threshold = otsu_threshold(img);

    // 2. Acha o topo geométrico exato procurando a transição de Otsu
    int topo_y = -1, topo_x = -1;
    for (int y = 1; y < H - 1; y++)
    {
        for (int x = 1; x < W - 1; x++)
        {
            int pixel_atual = img->pixels[y * W + x];
            int pixel_abaixo = img->pixels[(y + 1) * W + x];

            // Se cruzou o limiar de Otsu (fundo -> objeto ou objeto -> fundo), achou a borda superior
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

    // 3. Constrói o o grafo da imagem inteira
    Grafo *g = criar_grafo(img);

    // 4. Ativa um muro virtual que bloqueia o Dijkstra de atravessar a borda superior.
    // A borda começa em topo_y + 1 (pois topo_y é a última linha do fundo)
    int ref_y = topo_y + 1;
    if (ref_y >= H) ref_y = topo_y;
    
    // Descobre se o objeto é mais claro que o fundo
    int objeto_claro = (img->pixels[ref_y * W + topo_x] > threshold);

    // Escaneia a imagem de cima para baixo até achar o fim da borda
    int fundo_y = ref_y;
    while (fundo_y < H - 1)
    {
        int cor_atual = img->pixels[fundo_y * W + topo_x];
        int no_objeto = objeto_claro ? (cor_atual > threshold) : (cor_atual <= threshold);
        
        // Se a cor virar a cor do fundo, significa que atravessamos a borda e saímos por baixo
        if (!no_objeto) break; 
        fundo_y++;
    }

    // O muro vai do topo da tela até 2 pixels ANTES de sair da borda.
    // Isso deixa a borda inferior totalmente livre para o Dijkstra atravessar.
    int limite_muro = fundo_y - 2;
    if (limite_muro < topo_y + 1) limite_muro = topo_y + 1;

    for (int y = 0; y <= limite_muro; y++)
    {
        int id_esq = y * W + (topo_x - 1);
        int id_dir = y * W + topo_x;

        Aresta *a = g->lista_adj[id_esq];
        while (a)
        {
            if (a->destino % W >= topo_x)
                a->peso = 1e12; // Muro ativado
            a = a->prox;
        }

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

    // 6. Roda o Dijkstra
    Caminho c = dijkstra(g, origem, destino);

    if (c.tamanho > 0)
    {
        mostrar_caminho_ascii(img, &c);
        salvar_caminho_ppm("saida.ppm", img, &c);
    }
    else
    {
        printf("Caminho não encontrado\n");
    }

    liberar_caminho(&c);
    liberar_grafo(g);
    liberar_imagem(img);
    return 0;
}