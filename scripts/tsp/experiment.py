"""Definição e execução de experimentos.

Um experimento é uma pasta que responde a uma pergunta:

    experiments/<nome>/
        experiment.json   definição (entrada, vai pro git)
        runs/<id>/        uma pasta por execução do ga: config.json, points.csv, stats.csv, routes.csv
        summary.csv       uma linha por execução (gerado aqui, vai pro git)
        figures/          gráficos (vão pro git)

experiment.json:

    {
      "base":  {"n": 50, "pop": 200},                          flags fixas do ga, sem "--"
      "grid":  {"cross": ["ox", "obx"], "mut": [0.01, 0.05]},  produto cartesiano
      "problems": {"scenario": ["uniform", "circle"], "n": [50, 500]},  opcional
      "seeds": 30,                                             seeds do GA: N ou lista
      "instance_seeds": [1, 2, 3]                              mapas; padrão [1]
    }

Todas as configs rodam nos mesmos mapas e com as mesmas seeds, então os
resultados podem ser comparados em pares (ver compare.py).

"problems" é um produto cartesiano de instâncias (cenário, tamanho) cruzado com
o grid. Diferente do grid, não entra no rótulo `config`: a mesma configuração do
algoritmo é comparada em cada problema.

"grid" também pode ser uma lista de grids (a união deles), para parâmetros que
só fazem sentido com certos operadores:

    "grid": [{"sel": ["tournament"], "tournament": [2, 3, 5]},
             {"sel": ["roulette", "truncation"]}]
"""

from __future__ import annotations

import itertools
import json
import os
import subprocess
from concurrent.futures import ThreadPoolExecutor, as_completed
from dataclasses import dataclass
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
GA = ROOT / "build" / "ga"
EXPERIMENTS = ROOT / "experiments"

# Controlados pelo próprio experimento, não pela base/grid.
RESERVED = {"seed", "instance-seed", "out"}


@dataclass(frozen=True)
class Run:
    id: str
    config: str  # rótulo da combinação do grid, igual para todas as seeds
    dir: Path
    args: tuple[str, ...]  # argumentos do ga, sem --out


@dataclass
class Experiment:
    dir: Path
    definition: dict

    @property
    def name(self) -> str:
        return self.dir.name

    @property
    def figures_dir(self) -> Path:
        return self.dir / "figures"

    def plan(self) -> list[Run]:
        """Todas as execuções do experimento, em ordem determinística."""
        d = self.definition
        base = d.get("base", {})
        seeds = d.get("seeds", 30)
        seeds = list(range(seeds)) if isinstance(seeds, int) else list(seeds)
        instance_seeds = d.get("instance_seeds", [1])
        width = len(str(max(seeds)))

        runs: dict[str, Run] = {}
        for combo in _expand(d.get("grid")):
            prefix = _label(combo) or "base"
            for problem in _expand(d.get("problems")):
                params = {**base, **combo, **problem}
                stem = "_".join(filter(None, [prefix, _label(problem)]))
                for inst in instance_seeds:
                    for seed in seeds:
                        run_id = f"{stem}_i{inst}_s{seed:0{width}d}"
                        args = [
                            a for k, v in params.items() for a in (f"--{k}", str(v))
                        ]
                        args += ["--instance-seed", str(inst), "--seed", str(seed)]
                        runs[run_id] = Run(
                            run_id, prefix, self.dir / "runs" / run_id, tuple(args)
                        )
        return list(runs.values())


def resolve(path: str | Path | Experiment) -> Path:
    """Caminho existente ou nome solto, que vira experiments/<nome>."""
    if isinstance(path, Experiment):
        return path.dir
    p = Path(path)
    if p.is_absolute() or p.exists():
        return p.resolve()
    return EXPERIMENTS / p


def load(path: str | Path | Experiment) -> Experiment:
    if isinstance(path, Experiment):
        return path
    exp_dir = resolve(path)
    definition = json.loads((exp_dir / "experiment.json").read_text())
    return Experiment(exp_dir, definition)


def define(
    path: str | Path,
    *,
    base: dict | None = None,
    grid: dict | list | None = None,
    problems: dict | None = None,
    seeds: int | list[int] = 30,
    instance_seeds: list[int] | None = None,
) -> Experiment:
    """Grava experiment.json. Execuções cujos argumentos mudaram são refeitas no próximo run()."""
    definition = {
        "base": base or {},
        "grid": grid or {},
        "seeds": seeds,
        "instance_seeds": instance_seeds or [1],
    }
    if problems:
        definition["problems"] = problems
    _validate(definition)
    exp_dir = resolve(path)
    exp_dir.mkdir(parents=True, exist_ok=True)
    (exp_dir / "experiment.json").write_text(json.dumps(definition, indent=2) + "\n")
    return Experiment(exp_dir, definition)


def run(
    path: str | Path | Experiment,
    *,
    base: dict | None = None,
    grid: dict | list | None = None,
    problems: dict | None = None,
    seeds: int | list[int] | None = None,
    instance_seeds: list[int] | None = None,
    jobs: int | None = None,
    ga: str | Path | list = GA,
    force: bool = False,
    quiet: bool = False,
    shard: tuple[int, int] | None = None,
) -> Experiment:
    """Roda as execuções que faltam e regrava summary.csv.

    Com base/grid/problems/seeds, (re)define o experimento antes. Execuções já
    concluídas com os mesmos argumentos são puladas, então chamar de novo é barato.
    shard=(i, k) roda só a parte i de k (ver split()), para dividir entre máquinas.
    """
    if any(x is not None for x in (base, grid, problems, seeds)):
        exp = define(
            path,
            base=base,
            grid=grid,
            problems=problems,
            seeds=30 if seeds is None else seeds,
            instance_seeds=instance_seeds,
        )
    else:
        exp = load(path)
        _validate(exp.definition)

    # ga pode ser uma lista (ex.: [python, fake_ga.py] nos testes).
    if not isinstance(ga, list) and not Path(ga).exists():
        raise FileNotFoundError(f"{ga} não existe; compile com `cmake --build build`")
    command = [str(a) for a in ga] if isinstance(ga, list) else [str(ga)]

    plan = exp.plan() if shard is None else split(exp.plan(), *shard)
    todo = [r for r in plan if force or not _is_done(r)]
    if not quiet:
        print(f"{exp.name}: {len(plan)} execuções, {len(plan) - len(todo)} já prontas")

    failures: list[tuple[Run, str]] = []
    with ThreadPoolExecutor(jobs or os.cpu_count()) as pool:
        futures = {pool.submit(_execute, r, command): r for r in todo}
        for k, fut in enumerate(as_completed(futures), 1):
            if (err := fut.result()) is not None:
                failures.append((futures[fut], err))
            if not quiet and (k == len(todo) or k % max(1, len(todo) // 50) == 0):
                print(f"\r{exp.name}: {k}/{len(todo)}", end="", flush=True)
    if todo and not quiet:
        print()

    from . import load as loader  # evita import circular

    loader.build_summary(exp).to_csv(exp.dir / "summary.csv", index=False)

    if failures:
        run_, err = failures[0]
        raise RuntimeError(
            f"{len(failures)} execuções falharam; a primeira ({run_.id}):\n{err}"
        )
    return exp


def split(plan: list[Run], i: int, k: int) -> list[Run]:
    """Parte i (0 <= i < k) do plano, com custo estimado (n · pop · épocas)
    parecido entre as k partes. Determinística: toda máquina calcula a mesma divisão."""
    if not 0 <= i < k:
        raise ValueError(f"shard {i}/{k}: precisa de 0 <= i < k")
    totals = [0.0] * k
    parts: list[list[Run]] = [[] for _ in range(k)]
    for r in sorted(plan, key=_cost, reverse=True):
        j = totals.index(min(totals))
        totals[j] += _cost(r)
        parts[j].append(r)
    return parts[i]


def _cost(run: Run) -> float:
    args = dict(zip(run.args[::2], run.args[1::2]))
    return (
        float(args.get("--n", 50))
        * float(args.get("--pop", 200))
        * float(args.get("--max-epochs", 1000))
    )


def _label(combo: dict) -> str:
    return "_".join(f"{k}-{_slug(v)}" for k, v in combo.items())


def _expand(grid: dict | list | None) -> list[dict]:
    if not grid:
        return [{}]
    if isinstance(grid, list):
        return [combo for g in grid for combo in _expand(g)]
    keys = list(grid)
    return [dict(zip(keys, values)) for values in itertools.product(*grid.values())]


def _validate(definition: dict) -> None:
    grids = definition.get("grid") or {}
    keys = set(definition.get("base", {}))
    problems = definition.get("problems") or {}
    for g in [*(grids if isinstance(grids, list) else [grids]), problems]:
        keys |= set(g)
        for k, v in g.items():
            if not isinstance(v, list):
                raise ValueError(f"grid[{k!r}] precisa ser uma lista, recebeu {v!r}")
    if bad := keys & RESERVED:
        raise ValueError(
            f"{sorted(bad)} são controladas pelo experimento (use seeds/instance_seeds)"
        )


def _slug(value) -> str:
    return str(value).replace("/", "-").replace(" ", "")


def _is_done(run: Run) -> bool:
    marker = run.dir / "command.json"
    return marker.exists() and json.loads(marker.read_text()) == list(run.args)


def _execute(run: Run, command: list[str]) -> str | None:
    """Roda uma execução; retorna o stderr se falhar."""
    marker = run.dir / "command.json"
    marker.unlink(missing_ok=True)  # se falhar no meio, não fica marcada como pronta
    result = subprocess.run(
        [*command, *run.args, "--out", str(run.dir)], capture_output=True, text=True
    )
    if result.returncode != 0:
        return result.stderr.strip() or f"código de saída {result.returncode}"
    marker.write_text(json.dumps(list(run.args)) + "\n")
    return None
