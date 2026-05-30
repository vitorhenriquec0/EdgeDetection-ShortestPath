#include "min_heap.h"
#include <stdlib.h>

// Troca dois elementos e mantém pos[] consistente
static void swap(MinHeap *h, int a, int b) {
    HeapItem tmp  = h->data[a];
    h->data[a]    = h->data[b];
    h->data[b]    = tmp;

    // atualiza o mapa de posições dos dois nós trocados
    h->pos[h->data[a].no] = a;
    h->pos[h->data[b].no] = b;
}

/* Sobe o elemento na posição i até restaurar a propriedade
 * pai.dist <= filho.dist  (heapify-up).
 * Complexidade: O(log n) */
static void sobe(MinHeap *h, int i) {
    while (i > 0) {
        int pai = (i - 1) / 2;
        if (h->data[pai].dist <= h->data[i].dist)
            break;          /* propriedade restaurada */
        swap(h, pai, i);
        i = pai;
    }
}

/* Desce o elemento na posição i até restaurar a propriedade
 * (heapify-down). Escolhe sempre o filho de menor distância.
 * Complexidade: O(log n) */#include <stdio.h>
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
static void desce(MinHeap *h, int i) {
    while (1) {
        int menor = i;
        int esq   = 2 * i + 1;
        int dir   = 2 * i + 2;

        if (esq < h->tamanho &&
            h->data[esq].dist < h->data[menor].dist)
            menor = esq;

        if (dir < h->tamanho &&
            h->data[dir].dist < h->data[menor].dist)
            menor = dir;

        if (menor == i)
            break;          /* propriedade restaurada */

        swap(h, i, menor);
        i = menor;
    }
}


/*
 * heap_init — prepara a heap para uso.
 * total_nos deve ser LARGURA * ALTURA da imagem.
 * Marca todos os nós como "fora da heap" (pos = -1).
 */
void heap_init(MinHeap *h, int total_nos) {
    h->tamanho = 0;
    for (int i = 0; i < total_nos; i++)
        h->pos[i] = -1;   /* -1 significa: nó não está na heap */
}

/*
 * heap_inserir — adiciona o nó 'no' com distância 'dist'.
 * O elemento é inserido no fim e sobe até a posição correta.
 * Complexidade: O(log n)
 */
void heap_inserir(MinHeap *h, int no, double dist) {
    int i = h->tamanho++;    /* ocupa a próxima posição livre */

    h->data[i].no   = no;
    h->data[i].dist = dist;
    h->pos[no]      = i;

    sobe(h, i);              /* restaura a propriedade da heap */
}

/*
 * heap_remover_min — remove e retorna o nó de menor distância.
 * O último elemento ocupa a raiz e desce até a posição correta.
 * Complexidade: O(log n)
 *
 * ATENÇÃO: só chame se !heap_vazia(h).
 */
HeapItem heap_remover_min(MinHeap *h) {
    HeapItem minimo = h->data[0];    /* guarda o mínimo para retornar */

    h->tamanho--;

    if (h->tamanho > 0) {
        /* move o último elemento para a raiz */
        h->data[0]          = h->data[h->tamanho];
        h->pos[h->data[0].no] = 0;

        desce(h, 0);         /* restaura a propriedade da heap */
    }

    h->pos[minimo.no] = -1;  /* marca o nó como removido */
    return minimo;
}

void heap_diminuir_chave(MinHeap *h, int no, double nova_dist) {
    int i = h->pos[no];

    if (i == -1)
        return;              /* nó não está na heap, nada a fazer */

    if (nova_dist >= h->data[i].dist)
        return;              /* nova distância não é menor, ignora */

    h->data[i].dist = nova_dist;
    sobe(h, i);              /* nó pode ter subido na heap */
}

int heap_vazia(const MinHeap *h) {
    return h->tamanho == 0;
}

int heap_contem(const MinHeap *h, int no) {
    return h->pos[no] != -1;
}

MinHeap *criar_heap(int total_nos) {
    MinHeap *h = (MinHeap *)malloc(sizeof(MinHeap));
    if (!h) return NULL;
    
    // Aloca a memória exata para a imagem atual
    h->data = (HeapItem *)malloc(total_nos * sizeof(HeapItem));
    h->pos = (int *)malloc(total_nos * sizeof(int));
    
    if (!h->data || !h->pos) {
        free(h->data);
        free(h->pos);
        free(h);
        return NULL;
    }
    
    h->capacidade = total_nos;
    heap_init(h, total_nos); 
    return h;
}

void inserir_heap(MinHeap *h, int no, double dist) {
    if (heap_contem(h, no)) {
        heap_diminuir_chave(h, no, (float)dist); 
    } else {
        heap_inserir(h, no, (float)dist);
    }
}

ItemHeap remover_min(MinHeap *h) {
    return heap_remover_min(h); 
}

void liberar_heap(MinHeap *h) {
    if (h) {
        free(h->data); // Libera o array de itens
        free(h->pos);  // Libera o mapa de posições
        free(h);       // Libera a estrutura
    }
}