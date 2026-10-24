# Traveling-Salesman-Problem-using-Genetic-Algorithm
Solver em C++ (`src/`, `include/`), análise em Python (`scripts/tsp/`).

## Compilar e rodar uma execução

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/ga --help
./build/ga --n 50 --cross ox --mutation inversion --instance-seed 1 --seed 3 --out results/run
```

Cada execução grava em `--out`: `config.json`, `points.csv`, `stats.csv` e `routes.csv`.
`--instance-seed` sorteia o mapa; `--seed` controla só o algoritmo genético.

## Experimentos

```bash
pip install -r requirements.txt          # pandas, matplotlib, scipy e o pacote tsp (scripts/)
python scripts/run_experiment.py experiments/operators --jobs 8
```

Um experimento é uma pasta em `experiments/` que responde a uma pergunta:

```
experiments/operators/
  experiment.json   o que varia (grid), o que é fixo (base), seeds e mapas
  runs/             uma pasta por execução (fora do git)
  summary.csv       uma linha por execução: parâmetros + resultado final
  figures/          gráficos padrão
```

```json
{
  "base":  {"n": 50, "pop": 200, "sel": "tournament"},
  "grid":  {"cross": ["ox", "obx", "uniform"], "mutation": ["swap", "inversion"]},
  "seeds": 30,
  "instance_seeds": [1, 2, 3]
}
```

As chaves são as flags do `ga` sem `--`. Todas as configurações rodam nos mesmos mapas e
seeds, então dá para compará-las em pares. Execuções já concluídas são puladas, então mudar o
grid só roda as combinações novas.

A análise fica em `notebooks/analysis.ipynb`: tabela por configuração (`compare.table`),
convergência, boxplot, heatmap, testes de Friedman e Wilcoxon (`compare.friedman`,
`compare.pairwise`) e evolução da melhor rota.

## Testes

```bash
pytest tests
```

`tests/fake_ga.py` substitui o solver nos testes dos scripts enquanto ele não existe.
