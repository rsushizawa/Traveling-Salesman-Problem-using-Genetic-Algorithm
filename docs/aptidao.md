# Função de aptidão

## Matriz de distâncias

As cidades são os vértices de um grafo completo não direcionado; o peso da aresta
`(i, j)` é a distância euclidiana entre os pontos:

```
d(i, j) = sqrt((x_i - x_j)² + (y_i - y_j)²)
```

A matriz `n × n` é calculada **uma única vez**, antes da primeira geração
(`DistanceMatrix` em `include/distance.hpp`). Como o algoritmo avalia milhares de
rotas por época, consultar uma tabela é bem mais barato que recalcular raízes
quadradas a cada avaliação.

- **Simétrica:** `d(i, j) = d(j, i)`, então só a metade superior é calculada e espelhada.
- **Diagonal zero:** `d(i, i) = 0`.
- **Contígua:** um único `std::vector<double>` com índice `i*n + j`, em vez de
  `vector<vector<double>>`, para melhor uso de cache.

## Aptidão

Um indivíduo é uma permutação `r` de `0..n-1` (a ordem de visita). A aptidão é o
comprimento do ciclo, **incluindo a volta** da última cidade para a primeira:

```
L(r) = Σ_{k=0}^{n-2} d(r_k, r_{k+1})  +  d(r_{n-1}, r_0)
```

Implementada em `routeLength(route, dist)`.

**Minimizamos `L` diretamente**: menor comprimento = indivíduo mais apto.

### Por que não `1/L`?

Inverter só seria necessário se algum operador exigisse "maior é melhor". Usar
`L` direto:

- mantém a aptidão na mesma unidade da resposta (distância), o que facilita
  comparar com o ótimo conhecido do círculo (`2·n·r·sin(π/n)`) e plotar o erro relativo;
- não distorce as diferenças entre indivíduos (`1/L` comprime rotas longas);
- funciona sem adaptação no torneio e no truncamento, que só comparam/ordenam.

A roleta, que precisa de pesos "maior é melhor", converte localmente
(por exemplo, `L_max - L_i` ou `1/L_i`) sem mudar a aptidão armazenada.

### Cache

`Individual::length` guarda `L(r)` para não recalcular a cada comparação. Os
operadores não calculam aptidão: quem atualiza `length` depois de crossover e
mutação é o solver.
