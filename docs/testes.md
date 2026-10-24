# Suíte de Testes Unitários

Este documento descreve a infraestrutura de testes automatizados do projeto desenvolvida para validar os operadores genéticos (seleção, mutação e crossover). Os testes são integrados via **CMake** e **CTest**.

---

## Como Executar os Testes

Os testes podem ser compilados e executados a partir do diretório raiz do projeto:

```bash
# 1. Configurar e compilar o projeto com testes habilitados
cmake -B build
cmake --build build

# 2. Executar todos os testes via CTest
ctest --test-dir build --output-on-failure

# Ou executar os binários individuais diretamente:
./build/test_selection
./build/test_mutation
./build/test_crossover
```

---

## Estrutura dos Testes

### 1. Testes de Seleção (`tests/test_selection.cpp`)
Valida as propriedades estocásticas e determinísticas das estratégias de seleção:
- **`testAlphaSelect`:** 
  - Garante a seleção estrita do indivíduo com menor `length`.
  - Testa casos onde o melhor elemento está na primeira posição, no meio ou na última posição do vetor.
- **`testTournamentSelect`:**
  - Verifica integridade dos índices retornados com $k=1$.
  - Avalia a convergência seletiva para indivíduos de menor `length` à medida que o tamanho do torneio $k$ aumenta.
- **`testRouletteSelect`:**
  - Realiza $10.000$ iterações de amostragem em uma população com aptidões assimétricas.
  - Comprova empiricamente que as contagens de seleção obedecem à proporção inversa do comprimento da rota ($P_i \propto 1 / \text{length}_i$).
- **`testTruncationSelect`:**
  - Verifica se todas as seleções sorteadas pertencem estritamente à faixa do top `fraction` especificada (ex.: top 20%, top 30%).
  - Valida o caso de borda onde a fração calculada resulta em menos de um indivíduo, assegurando que ao menos a melhor rota seja preservada.

---

### 2. Testes de Mutação (`tests/test_mutation.cpp`)
Valida os invariantes de permutação e integridade dos indivíduos modificados:
- **Preservação de Permutação (`isValidPermutation`):**
  - Todas as mutações (`swapMutation`, `insertionMutation`, `scrambleMutation`, `inversionMutation`) são submetidas a dezenas de repetições.
  - Verifica se o vetor resultante contém exatamente uma ocorrência de cada número no intervalo $0 \dots n-1$.
- **Invariante Específico do Swap:**
  - Assegura que a mutação por troca altera exatamente duas posições no vetor da rota.
- **Casos de Borda (*Edge Cases*):**
  - Testa comportamento seguro e idempotente com rotas vazias ($n=0$) e rotas com um único vértice ($n=1$), sem causar falhas de segmentação.

---

### 3. Testes de Crossover (`tests/test_crossover.cpp`)
Valida a geração de descendentes válidos a partir de combinações parentais:
- **Operadores Avaliados:**
  - `orderedCrossover` (OX)
  - `orderBasedCrossover` (OBX)
  - `uniformCrossoverRepaired` (Uniforme com Reparo)
- **Critérios de Validação:**
  - **Validade do Filho:** Confirma que todos os operadores geram filhos que são permutações válidas de tamanho $n$.
  - **Imutabilidade dos Pais:** Garante que as rotas dos pais passadas como entrada permanecem inalteradas após a operação.
  - **Casos de Borda:** Testa combinações parentais com tamanhos $n=0$ e $n=1$.
