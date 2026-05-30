#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "grafo.h"
#include "dijkstra.h"

int main(void)
{
    Imagem *img = ler_pgm("image.pgm");
    if (!img) return 1;

    int W = img->largura;
    int H = img->altura;

    // 1. Achar o topo do objeto (pela transição de cor, não pela cor absoluta)
    int topo_y = -1, topo_x = -1;
    
    // Começa de y=1 e x=1 para ignorar a borda extrema da imagem
    for (int y = 1; y < H - 1; y++) {
        for (int x = 1; x < W / 2; x++) {
            int v_esq   = img->pixels[y * W + (x - 1)];
            int v_dir   = img->pixels[y * W + (x + 1)];
            int v_cima  = img->pixels[(y - 1) * W + x];
            int v_baixo = img->pixels[(y + 1) * W + x];
            
            // Calcula o gradiente para achar a verdadeira borda do desenho
            int mag = abs(v_dir - v_esq) + abs(v_baixo - v_cima);
            
            // Se houver uma quebra brusca de cor (fundo para o objeto), achamos o topo real!
            if (mag > 50) { 
                topo_y = y;
                topo_x = x;
                break;
            }
        }
        if (topo_y != -1) break; // Sai da busca assim que achar o primeiro topo
    }

    if (topo_y == -1 || topo_x <= 0) {
        fprintf(stderr, "[ERRO] Não foi possível encontrar a borda do objeto.\n");
        liberar_imagem(img);
        return 1;
    }

    // Constrói o grafo (vai usar a heurística do grafo.c)
    Grafo *g = criar_grafo(img);

    // 2. Levantar o muro intransponível (Blindagem Total)
    for (int y = 0; y <= topo_y; y++) {
        int id_esq = y * W + (topo_x - 1);
        int id_dir = y * W + topo_x;

        Aresta *a = g->lista_adj[id_esq];
        while(a) {
            // Qualquer vizinho que esteja na coluna do muro ou à direita é bloqueado
            if (a->destino % W >= topo_x) a->peso = 1e12;
            a = a->prox;
        }
        
        a = g->lista_adj[id_dir];
        while(a) {
            // Qualquer vizinho que esteja à esquerda do muro é bloqueado
            if (a->destino % W <= topo_x - 1) a->peso = 1e12;
            a = a->prox;
        }
    }

    // 3. Origem e Destino são vizinhos separados pelo muro
    int origem  = topo_y * W + (topo_x - 1); 
    int destino = topo_y * W + topo_x;       

    printf("Topo detectado: (%d, %d)\n", topo_x, topo_y);
    printf("Origem  (Esq do muro): %d\n", origem);
    printf("Destino (Dir do muro): %d\n", destino);

    // 4. Roda o Dijkstra forçado a dar a volta no objeto
    Caminho c = dijkstra(g, origem, destino);

    if (c.tamanho > 0) {
        printf("Caminho encontrado: %d pixels\n", c.tamanho);
        mostrar_caminho_ascii(img, &c);
        salvar_caminho_ascii("saida_ascii.txt", img, &c);
        salvar_caminho_ppm("saida.ppm", img, &c);
    } else {
        printf("[AVISO] Caminho não encontrado\n");
    }

    liberar_caminho(&c);
    liberar_grafo(g);
    liberar_imagem(img);
    return 0;
}