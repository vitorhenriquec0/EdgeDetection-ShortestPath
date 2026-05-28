# Detecao de Bordas em Exames por Menor Caminho

## Visao geral

Este projeto propõe uma versao simplificada de segmentacao de contornos em imagens medicas usando grafos e o algoritmo de Dijkstra. A ideia e tratar a imagem como um grafo, onde cada pixel representa um no, e usar pesos nas arestas para favorecer o caminho que se ajusta melhor ao contorno de estruturas como ossos, pulmoes ou tumores.

Em vez de depender de algoritmos mais complexos de fluxo maximo, a solucao foca em uma implementacao direta em C, com estruturas de dados classicas da disciplina de Estruturas de Dados.

## Como a abordagem funciona

1. A imagem e o grafo: cada pixel vira um no do grafo.
2. As arestas conectam cada pixel aos seus 8 vizinhos.
3. O peso da aresta depende da diferenca de intensidade entre pixels vizinhos.
4. Quanto maior a diferenca entre dois pixels, menor o peso, o que faz o algoritmo preferir caminhos sobre bordas e contornos.
5. O usuario escolhe um ponto inicial e um ponto final, e o Dijkstra encontra o caminho minimo entre eles.
6. Esse caminho e desenhado na imagem final para evidenciar o contorno detectado.

Uma formula simples para os pesos pode ser:

```text
peso = 1 / (|I1 - I2| + 1)
```

Assim, regiao homogenea tende a ter custo maior, enquanto transicoes bruscas de intensidade tendem a ser mais atraentes para o caminho minimo.

## Divisao dos modulos

Para manter o trabalho organizado, o projeto pode ser dividido em cinco partes independentes:

### 1. Leitor de arquivos PGM/PPM

- Abre arquivos PGM, que armazenam imagens em tons de cinza de forma simples.
- Carrega os valores dos pixels em uma matriz `int imagem[ALTURA][LARGURA]`.
- Pode ser expandido para suporte a PPM, se necessario.

### 2. Construtor do grafo

- Transforma a matriz de pixels em uma lista de adjacencia.
- Conecta cada pixel aos seus 8 vizinhos.
- Calcula os pesos das arestas com base na diferenca de intensidade.

### 3. Dijkstra

- Implementa o algoritmo de Dijkstra em C puro.
- Recebe o ponto inicial e o ponto final.
- Retorna o caminho minimo como uma sequencia de coordenadas.

### 4. Fila de prioridade

- Implementa um Min-Heap do zero.
- Garante que o Dijkstra rode de forma eficiente.
- Inclui operacoes como insercao, remocao do menor elemento e diminuicao de chave.

### 5. Interface e resultados

- Gera uma nova imagem com o caminho encontrado destacado.
- Pinta os pixels do contorno em vermelho para facilitar a apresentacao.
- Produz o resultado visual final para comparacao com a imagem original.
