# Operadores de Crossover para Permutação

Este documento descreve os operadores de crossover implementados para rotas de permutação (problemas do tipo Caixeiro Viajante - TSP). Operadores possuem complexidade temporal $O(N)$ e evitam busca linear utilizando tabelas de acesso direto com `std::vector<bool>`.

## Operadores Implementados

### 1. Ordered Crossover (OX)
- **Descrição:** Preserva a ordem contígua e os blocos de adjacência de um segmento do primeiro pai, preenchendo o restante das posições com a ordem relativa do segundo pai.
- **Funcionamento:**
  1. Define um intervalo aleatório $[cut1, cut2]$.
  2. Copia o segmento $a[cut1 \dots cut2]$ diretamente para o filho.
  3. Preenche as posições restantes de forma circular a partir de $cut2 + 1$ utilizando os elementos do pai $b$, ignorando os genes já presentes.

### 2. Order-Based Crossover (OBX)
- **Descrição:** Preserva as posições de genes selecionados aleatoriamente do primeiro pai e preenche os demais mantendo a ordem relativa em que aparecem no segundo pai.
- **Funcionamento:**
  1. Seleciona posições de $a$ com probabilidade de 50% (garantindo ao menos uma posição selecionada).
  2. Mapeia quais cidades estão nessas posições.
  3. Mantém os genes não selecionados fixos no filho e preenche as posições selecionadas com as cidades correspondentes na ordem em que aparecem no pai $b$.

### 3. Uniform Crossover Repaired
- **Descrição:** Combina os genes dos pais posição por posição com probabilidade uniforme (50%), aplicando um algoritmo de reparo para resolver duplicatas e omissões.
- **Funcionamento:**
  1. Para cada posição $i$, escolhe $a[i]$ ou $b[i]$ via distribuição de Bernoulli.
  2. Caso o gene sorteado já exista no filho, a posição é marcada como vazia.
  3. Identifica todas as cidades ausentes, embaralha-as aleatoriamente (`std::shuffle`) e preenche as posições vazias.