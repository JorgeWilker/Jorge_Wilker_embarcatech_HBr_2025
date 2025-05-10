# 📊 Galton Board Concept

## Conceito Físico

O Galton Board (ou Quincunx) é um dispositivo inventado por Sir Francis Galton para demonstrar a distribuição binomial e o teorema do limite central.

O dispositivo consiste em uma placa vertical com pinos dispostos em uma matriz triangular. Bolinhas são lançadas do topo e, à medida que elas caem, colidem com os pinos, tendo 50% de chance de ir para a esquerda ou direita em cada colisão.

## Diagrama Simplificado

Abaixo está uma representação simplificada de um Galton Board com 4 níveis:

```
                    O                     (Entrada da bola)
                   / \
                  /   \
                 O     O                  (Nível 1)
                / \   / \
               /   \ /   \
              O     O     O              (Nível 2)
             / \   / \   / \
            /   \ /   \ /   \
           O     O     O     O          (Nível 3)
          / \   / \   / \   / \
         /   \ /   \ /   \ /   \
        O     O     O     O     O      (Nível 4)
        |     |     |     |     |
       [0]   [1]   [2]   [3]   [4]      (Bins de coleta)
```

## Representação da Distribuição

Após muitas bolinhas caírem, a distribuição nos bins se aproxima de uma distribuição normal:

```
    |                                  |
    |                 █                |
    |                 █                |
    |              █  █  █             |
    |              █  █  █             |
    |           █  █  █  █  █          |
    |           █  █  █  █  █          |
    |        █  █  █  █  █  █  █       |
    |        █  █  █  █  █  █  █       |
    |     █  █  █  █  █  █  █  █  █    |
    +----------------------------------+
      0  1  2  3  4  5  6  7  8  9  10   (Bins em um Galton Board de 10 níveis)
```

## Implementação no Projeto

Em nossa simulação digital:

1. Cada bola começa na posição 0 do topo.
2. A bola passa por `NUM_LEVELS` (10) níveis de decisão.
3. Em cada nível, existe 50% de chance da bola ir para a direita (incrementando sua posição) ou para a esquerda (mantendo sua posição).
4. Após passar por todos os níveis, a bola cai em um dos `NUM_BINS` (11) coletores.
5. O processo é repetido para `NUM_BALLS` (100) bolas.
6. A distribuição final demonstra visualmente a distribuição binomial/normal resultante.

## Significado Matemático

O Galton Board ilustra:

- A distribuição binomial (cada trajeto da bola segue uma distribuição de Bernoulli)
- O teorema do limite central (à medida que muitas bolas caem, a distribuição se aproxima de uma normal)
- Combinações e probabilidades (o número de caminhos possíveis para cada bin segue os coeficientes binomiais) 