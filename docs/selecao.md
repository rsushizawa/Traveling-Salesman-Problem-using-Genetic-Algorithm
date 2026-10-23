# Operadores de Seleção

Este documento descreve os operadores de seleção implementados para o Problema do Caixeiro Viajante (TSP) em `src/selection.cpp`, detalhando seus contratos, características algorítmicas, complexidade temporal e tratamento de estabilidade numérica.

---

## Contrato dos Operadores

Conforme definido em `include/selection.hpp`:
- **Retorno por Índice:** Todas as funções retornam o índice (`int`) do indivíduo selecionado dentro do vetor `const Population &pop`. Não são retornadas referências ou ponteiros diretos para evitar invalidação de memória durante a construção de novas gerações.
- **Função Objetivo:** O critério de avaliação baseia-se em `Individual::length`. No problema do TSP, trata-se de um problema de **minimização** (menor comprimento representa melhor aptidão).
- **Preservação de Estado:** O vetor de entrada `pop` é constante (`const Population&`), garantindo que a seleção não introduza efeitos colaterais na população corrente.
- **Aleatoriedade Controlada:** Toda estocasticidade é provida pelo gerador pseudoaleatório `Rng &rng` (`std::mt19937_64`) passado por referência.

---

## Operadores Implementados

### 1. Seleção por Torneio (`tournamentSelect`)
- **Descrição:** Seleciona o melhor indivíduo entre $k$ concorrentes sorteados aleatoriamente da população.
- **Funcionamento:**
  1. Sorteia o primeiro participante uniformemente entre $0$ e $N-1$ e toma seu comprimento como referência inicial.
  2. Sorteia sequencialmente mais $k - 1$ participantes via amostragem independente com reposição.
  3. Compara cada concorrente com o melhor atual e atualiza o vencedor sempre que encontrar um indivíduo com menor `length`.
- **Complexidade Temporal:** $\mathcal{O}(k)$ em tempo e $\mathcal{O}(1)$ em memória auxiliar, sem alocações dinâmicas na heap.
- **Pressão Seletiva:** Ajustável pelo parâmetro $k$. Valores maiores de $k$ aumentam a pressão seletiva sobre a população.

### 2. Seleção por Roleta / Aptidão Proporcional (`rouletteSelect`)
- **Descrição:** A probabilidade de seleção de cada indivíduo é inversamente proporcional ao seu custo de rota ($1 / \text{length}$).
- **Funcionamento:**
  1. Calcula a soma total dos valores de aptidão invertidos:
     $$\text{fitnessSum} = \sum_{i=0}^{N-1} \frac{1}{\text{pop}[i].\text{length}}$$
  2. Sorteia um valor de corte contínuo no intervalo $[0, \text{fitnessSum})$ via `std::uniform_real_distribution<double>`.
  3. Percorre a população acumulando a aptidão até atingir ou ultrapassar o ponto sorteado, retornando o índice correspondente.
- **Complexidade Temporal:** $\mathcal{O}(N)$ em tempo e $\mathcal{O}(1)$ em memória auxiliar (itera por referência constante sem cópias de rotas).

### 3. Seleção Alpha / Elitista (`alphaSelect`)
- **Descrição:** Identifica e seleciona deterministicamente o melhor indivíduo (menor `length`) de toda a população.
- **Funcionamento:**
  1. Percorre linearmente a população mantendo o registro do menor `length` encontrado e seu respectivo índice.
- **Complexidade Temporal:** $\mathcal{O}(N)$ em tempo e $\mathcal{O}(1)$ em memória auxiliar.
- **Aplicação:** Usado principalmente para elitismo estrito, preservação da melhor solução encontrada ou estratégias híbridas de reprodução.

### 4. Seleção por Truncamento (`truncationSelect`)
- **Descrição:** Filtra apenas a fração superior dos melhores indivíduos (a elite) e sorteia uniformemente um progenitor dentro desse grupo restrito.
- **Funcionamento:**
  1. Determina a quantidade limite de indivíduos:
     $$\text{limite} = \operatorname{clamp}(\lfloor N \times \text{fraction}\rfloor, 1, N)$$
  2. Inicializa um vetor de índices `0..N-1` utilizando `std::iota`.
  3. Aplica `std::partial_sort` para ordenar apenas os índices referentes ao top `limite` com base no `length` correspondente em `pop`.
  4. Sorteia um índice uniformemente dentro do intervalo $[0, \text{limite} - 1]$ via `std::uniform_int_distribution<size_t>`.
- **Vantagens de Implementação:** 
  - Não viola a constância de `pop` e não move indivíduos na memória (evita copiar/mover vetores de rotas).
  - Executa em tempo $\mathcal{O}(N \log(\text{limite}))$ em vez do custo $\mathcal{O}(N \log N)$ de uma ordenação completa.
