# Operadores de Mutação

Este documento descreve os operadores de mutação implementados para o Problema do Caixeiro Viajante (TSP) em `src/mutation.cpp`, suas características algorítmicas, garantias de tempo de execução e a discussão sobre o uso de janelas de mutação.

---

## Contrato dos Operadores

Conforme especificado em `include/mutation.hpp`:
- Modificam a rota (`Route &r`) **no lugar** (*in-place*).
- Preservam a validade da permutação dos vértices `0..n-1` (sem repetições ou perdas de cidades).
- Toda aleatoriedade é provida pelo gerador pseudoaleatório `Rng &rng` passado por referência.
- Não recalculam a função de aptidão (*fitness*); a atualização de `Individual::length` cabe ao módulo solver.
- Tratam casos triviais com guarda preventiva (`if (r.size() < 2) return;`).

---

## Sorteio em Tempo Constante $O(1)$ sem Rejeição

Tradicionalmente, a escolha de dois índices distintos $i \neq j$ em um vetor de tamanho $n$ é feita sorteando repetidamente até que os valores difiram:

```cpp
// Abordagem com rejeição (tempo indeterminado no pior caso):
int i = dist(rng);
int j = dist(rng);
while (i == j) {
    j = dist(rng);
}
```

Para garantir execução em **tempo estritamente determinístico e constante $O(1)$**, implementou-se o sorteio mapeado:
1. Sorteia-se $i \in [0, n - 1]$.
2. Sorteia-se $j' \in [0, n - 2]$ (um espaço com $n-1$ possibilidades).
3. Se $j' \ge i$, ajusta-se $j = j' + 1$; caso contrário, $j = j'$.

Dessa forma, $j$ nunca colide com $i$, todas as combinações de pares distintos possuem distribuição uniforme e elimina-se qualquer laço condicional (*busy-waiting*).

---

## Operadores Implementados

### 1. Swap Mutation (`swapMutation`)
- **Mecanismo:** Seleciona dois índices distintos $i$ e $j$ e troca as posições das cidades usando `std::swap(r[i], r[j])`.
- **Comportamento:** Introduz uma perturbação pontual e discreta, alterando até 4 arestas do ciclo.

### 2. Insertion Mutation (`insertionMutation`)
- **Mecanismo:** Seleciona uma cidade no índice de origem e a transfere para um índice de destino, deslocando o trecho intermediário via `std::rotate`.
- **Comportamento:** Preserva a ordem relativa de todos os demais vértices, apenas reposicionando uma parada ao longo do trajeto.

### 3. Inversion Mutation (`inversionMutation`)
- **Mecanismo:** Seleciona dois pontos de corte $i < j$ e inverte toda a subsequência $[i, j]$ utilizando `std::reverse`.
- **Comportamento no TSP:** É o operador clássico equivalente ao movimento **2-opt**. Ele substitui duas arestas cruzadas por duas arestas não cruzadas, sendo historicamente um dos operadores mais eficazes para problemas euclidianos de rotas.

### 4. Scramble Mutation (`scrambleMutation`)
- **Mecanismo:** Seleciona dois pontos de corte $i < j$ e embaralha aleatoriamente todos os elementos no intervalo $[i, j]$ usando `std::shuffle`.
- **Comportamento:** Introduz maior grau de estocasticidade e diversidade à população, quebrando blocos de subsequências estabelecidas.

---

## Discussão: Janela de Mutação como Parâmetro

### Cenário Atual
Na implementação atual, os pontos de corte para `inversionMutation` e `scrambleMutation` são sorteados uniformemente sobre todo o domínio $[0, n-1]$. Isso significa que a distância $|i - j|$ pode variar livremente de $1$ até $n-1$, podendo afetar quase a totalidade da rota em um único passo.

### Benefícios de uma Janela Parametrizável ($W_{\max}$)
Embora a especificação original do projeto não inclua um parâmetro de janela, limitar a extensão máxima do trecho modificado traz vantagens significativas:
1. **Preservação de Blocos Construtivos (*Building Blocks*):** Em instâncias grandes de TSP, inverter ou embaralhar $80\%$ da rota atua de maneira similar a um reinício aleatório destrutivo. Uma janela máxima (ex.: $W_{\max} \in [4, 15]$ cidades) foca a mutação em reorganizações puramente locais.
2. **Controle Fino de Exploração vs. Explotação:** 
   - Janelas maiores no início da busca auxiliam a explorar bacias de atração diferentes.
   - Janelas menores nas gerações finais atuam como refinamento local (*fine-tuning*).
3. **Parametrização Futura:** Caso seja incorporada à configuração do algoritmo (`Config`), o sorteio do segundo ponto poderia ser facilmente ajustado para:
   $$\text{tamanho} \in [2, \min(W_{\max}, n)], \quad \text{cut}_2 = \text{cut}_1 + \text{tamanho} - 1$$
