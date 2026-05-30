# Detecao de Bordas por Contorno Fechado

## Visao geral

Este projeto extrai o contorno fechado de um objeto de alto contraste em uma imagem PGM. A imagem e tratada como uma matriz de pixels; em seguida, o programa calcula um limiar automatico, testa o objeto como escuro e como claro, seleciona o maior componente conectado que nao encosta na borda da imagem e coleta os seus pixels de borda.

O resultado e um contorno completo destacado na imagem final, adequado para casos como um circulo preenchido de preto ou branco sobre fundo uniforme.

## Como a abordagem funciona

1. A imagem PGM e carregada em memoria como uma matriz de intensidades.
2. O programa calcula um limiar automatico pelo metodo de Otsu.
3. O algoritmo testa duas hipoteses: objeto escuro sobre fundo claro e objeto claro sobre fundo escuro.
4. Em cada hipotese, ele busca o maior componente conectado que nao encosta na borda da imagem.
5. Entre as duas hipoteses, escolhe a melhor componente e marca apenas os pixels de borda desse objeto.
6. Esses pixels sao desenhados na imagem final e tambem exportados em ASCII.

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

### 3. Extracao de contorno

- Calcula um limiar automatico para separar primeiro plano e fundo.
- Testa o objeto como escuro e como claro.
- Seleciona o maior componente interno e extrai os seus pixels de borda.

### 4. Visualizacao e resultados

- Gera uma nova imagem com o contorno destacado.
- Pinta os pixels do contorno em vermelho para facilitar a apresentacao.
- Produz tambem uma pre-visualizacao ASCII do resultado.

## Observacoes

- Funciona melhor para imagens com fundo uniforme e um objeto principal bem definido.
- Se o objeto encostar na borda da imagem ou houver muito ruido, a deteccao pode precisar de ajuste manual.
