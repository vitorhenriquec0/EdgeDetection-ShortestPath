# Detecao de Bordas por Contorno Fechado

## Visao geral

Este projeto extrai o contorno fechado de um objeto de alto contraste
em uma imagem PGM. A imagem e tratada em duas etapas: primeiro, o
programa calcula um limiar automatico pelo metodo de Otsu e localiza
o topo do objeto detectando a primeira transicao de intensidade;
depois, constroi o grafo completo da imagem, aplica um muro topologico
que bloqueia o caminho direto entre origem e destino, e usa o Dijkstra
para percorrer todo o contorno.

O resultado e um contorno completo destacado na imagem final, adequado
para casos como um circulo preenchido de preto ou branco sobre fundo
uniforme.

## Como a abordagem funciona

1. A imagem PGM e carregada em memoria como uma matriz de intensidades.
2. O programa calcula um limiar automatico pelo metodo de Otsu.
3. O topo geometrico do objeto e detectado procurando a primeira
   transicao de intensidade que cruza esse limiar, varrendo a imagem
   de cima para baixo.
4. O grafo completo da imagem e construido conectando cada pixel aos
   seus 8 vizinhos, com pesos calculados pelo gradiente local.
5. Um muro topologico e aplicado: arestas que cruzam a coluna do topo
   recebem peso altissimo (1e12), bloqueando o caminho direto entre
   origem e destino.
6. Origem e destino sao posicionados lado a lado no topo detectado,
   um de cada lado do muro.
7. O Dijkstra e executado e, forcado a desviar do muro, percorre todo
   o contorno do objeto.
8. Esse caminho e desenhado na imagem final e tambem exportado em ASCII.

## Funcao de peso

O peso de cada aresta e calculado a partir do gradiente local de
intensidade do pixel de destino:

```text
gx = I(x+1, y) - I(x-1, y)
gy = I(x, y+1) - I(x, y-1)
magnitude = |gx| + |gy|
peso = 1000000 / (magnitude^2 + 1)
```

Pixels de interior (baixa variacao de intensidade) resultam em
magnitude baixa e peso alto, o Dijkstra evita passar por ali.
Pixels de borda (alta variacao de intensidade) resultam em magnitude
alta e peso baixo, o Dijkstra prefere esse caminho.
O quadrado da magnitude amplifica essa diferenca, tornando a atracao
pelas bordas mais pronunciada do que com uma formula linear.
Nas extremidades da imagem, onde um vizinho nao existe, usa-se o
valor do proprio pixel central para evitar gradientes artificiais.

## Divisao dos modulos

### 1. Leitor de arquivos PGM (pgm.c)
- Abre arquivos PGM P2 (texto puro) e carrega os pixels em um
  array linear contíguo: pixels[y * largura + x].
- Exporta tambem funcoes para salvar PGM e liberar a imagem.

### 2. Construtor do grafo (grafo.c)
- Constroi o grafo completo da imagem com conectividade 8.
- Calcula o peso de cada aresta pela formula do gradiente acima.
- Exporta liberar_grafo para liberar toda a memoria alocada.

### 3. Algoritmo de caminho minimo (dijkstra.c)
- Implementa o Dijkstra com Min-Heap.
- Recebe o grafo, a origem e o destino; devolve o Caminho encontrado.
- Inclui verificacao de destino inalcancavel e reconstrucao do
  trajeto pelo vetor de predecessores.

### 4. Fila de prioridade (min_heap.c)
- Implementa um Min-Heap binario com mapa de posicoes.
- Suporta heap_inserir, heap_remover_min e heap_diminuir_chave,
  todos em O(log n).

### 5. Visualizacao e resultados (output.c)
- Gera imagem PPM com o contorno pintado em vermelho.
- Produz pre-visualizacao ASCII no terminal e em arquivo texto.

### 6. Programa principal (main.c)
- Orquestra todas as etapas: leitura, Otsu, deteccao do topo,
  construcao do grafo, muro topologico, Dijkstra e saida.

## Como compilar e executar

```bash
gcc main.c dijkstra.c grafo.c pgm.c output.c min_heap.c -o detect -lm
./detect imagem.pgm
```

Os arquivos de saida gerados sao:
- saida.ppm - imagem com o contorno em vermelho
- saida_ascii.txt - pre-visualizacao em texto

## Observacoes

- Funciona melhor para imagens com fundo uniforme e um objeto
  principal bem definido.
- Se o objeto encostar na borda da imagem, o muro topologico pode
  nao funcionar corretamente.
- Se houver muito ruido, o gradiente pode atrair o caminho para
  regioes erradas.