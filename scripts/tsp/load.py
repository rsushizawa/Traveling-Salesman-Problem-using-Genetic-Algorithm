from __future__ import annotations

import json
import math
import warnings
from pathlib import Path

import pandas as pd

from .experiment import Experiment, Run
from .experiment import load as load_experiment

# Colunas do summary que não são parâmetros da config.
METRICS = [
    "best",
    "final_mean",
    "epochs",
    "epoch_of_best",
    "elapsed_ms",
    "optimum",
    "gap_pct",
]
# Parâmetros que identificam a repetição, não um fator do experimento.
BLOCKS = ["instanceSeed", "seed"]

# Flag da linha de comando -> chave do config.json (manter igual ao src/cli.cpp).
FLAG_TO_COLUMN = {
    "scenario": "scenario",
    "n": "n",
    "side": "side",
    "radius": "radius",
    "pop": "populationSize",
    "cross-rate": "crossoverRate",
    "mut": "mutationRate",
    "cross": "crossover",
    "mutation": "mutation",
    "sel": "selection",
    "tournament": "tournamentSize",
    "truncation": "truncationFraction",
    "elitism": "elitism",
    "max-epochs": "maxEpochs",
    "stagnation": "stagnation",
    "route-every": "routeEvery",
}


def circle_optimum(n: int, r: float) -> float:
    """Igual ao circleOptimum do C++: 2·n·r·sin(π/n)."""
    return 2 * n * r * math.sin(math.pi / n)


def config(run_dir: Path | Run) -> dict:
    cfg = json.loads((_dir(run_dir) / "config.json").read_text())
    cfg.pop("outDir", None)
    return cfg


def stats(run_dir: Path | Run) -> pd.DataFrame:
    return pd.read_csv(_dir(run_dir) / "stats.csv")


def points(run_dir: Path | Run) -> pd.DataFrame:
    return pd.read_csv(_dir(run_dir) / "points.csv", index_col="id")


def routes(run_dir: Path | Run) -> pd.DataFrame:
    """epoch, route (lista de ints)."""
    df = pd.read_csv(_dir(run_dir) / "routes.csv", dtype={"route": str})
    df["route"] = df["route"].map(lambda s: [int(c) for c in s.split()])
    return df


def build_summary(exp: Experiment) -> pd.DataFrame:
    """Uma linha por execução concluída: parâmetros da config + métricas finais."""
    rows = [
        _summary_row(exp, r) for r in exp.plan() if (r.dir / "command.json").exists()
    ]
    df = pd.DataFrame(rows)
    if df.empty:
        return df
    first = ["experiment", "run", "config", *factors(exp, df), *BLOCKS]
    return df[first + [c for c in df.columns if c not in first]]


def summary(*experiments: str | Path | Experiment) -> pd.DataFrame:
    """summary.csv de um ou mais experimentos, concatenados."""
    frames = []
    for exp in map(load_experiment, experiments):
        path = exp.dir / "summary.csv"
        df = pd.read_csv(path) if path.exists() else build_summary(exp)
        if df.empty:
            warnings.warn(
                f"{exp.name}: nenhuma execução concluída; rode experiment.run()"
            )
        frames.append(df)
    return pd.concat(frames, ignore_index=True)


def history(
    *experiments: str | Path | Experiment, columns: list[str] | None = None
) -> pd.DataFrame:
    """stats.csv de todas as execuções em formato longo (uma linha por época).

    Cada linha leva experiment, run, as colunas pedidas (padrão: as que variam)
    e as seeds, para agrupar nos gráficos de convergência.
    """
    exps = [load_experiment(e) for e in experiments]
    summ = summary(*exps)
    keep = ["experiment", "run", "config", *(columns or varied_columns(summ)), *BLOCKS]
    info = summ[keep].set_index(["experiment", "run"])

    frames = []
    for exp in exps:
        for r in exp.plan():
            if (exp.name, r.id) not in info.index:
                continue
            st = stats(r)
            for col, value in zip(keep, (exp.name, r.id, *info.loc[(exp.name, r.id)])):
                st[col] = value
            frames.append(st)
    return pd.concat(frames, ignore_index=True)


def varied_columns(df: pd.DataFrame) -> list[str]:
    """Parâmetros que mudam entre as execuções (os fatores do experimento)."""
    skip = {"experiment", "run", "config", *METRICS, *BLOCKS}
    return [c for c in df.columns if c not in skip and df[c].nunique(dropna=False) > 1]


def factors(exp: str | Path | Experiment, df: pd.DataFrame | None = None) -> list[str]:
    """Colunas que variam, na ordem em que aparecem no grid do experimento."""
    exp = load_experiment(exp)
    df = summary(exp) if df is None else df
    grids = exp.definition.get("grid") or {}
    problems = exp.definition.get("problems") or {}
    order = [
        FLAG_TO_COLUMN.get(k, k)
        for g in [*(grids if isinstance(grids, list) else [grids]), problems]
        for k in g
    ]
    varied = varied_columns(df)
    return sorted(varied, key=lambda c: order.index(c) if c in order else len(order))


def problem_factors(exp: str | Path | Experiment) -> list[str]:
    """Colunas que vêm de "problems" (cenário, tamanho), em vez do grid."""
    exp = load_experiment(exp)
    return [FLAG_TO_COLUMN.get(k, k) for k in exp.definition.get("problems") or {}]


def _dir(run_dir: Path | Run) -> Path:
    return run_dir.dir if isinstance(run_dir, Run) else Path(run_dir)


def _summary_row(exp: Experiment, run: Run) -> dict:
    cfg = config(run)
    row = {"experiment": exp.name, "run": run.id, "config": run.config, **cfg}

    st = stats(run)
    best = math.nan
    if not st.empty:
        best = st["best"].min()
        row.update(
            best=best,
            final_mean=st["mean"].iloc[-1],
            epochs=int(st["epoch"].iloc[-1]) + 1,
            epoch_of_best=int(st.loc[st["best"].idxmin(), "epoch"]),
            elapsed_ms=st["elapsed_ms"].iloc[-1],
        )
    if cfg["scenario"] == "circle":
        row["optimum"] = circle_optimum(cfg["n"], cfg["radius"])
        row["gap_pct"] = 100 * (best - row["optimum"]) / row["optimum"]
    return row
