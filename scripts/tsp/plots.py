from __future__ import annotations

from contextlib import contextmanager
from pathlib import Path

import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

from . import load
from .experiment import Experiment, Run
from .experiment import load as load_experiment

# Cores padrão do matplotlib: ciclo tab10 e colormap viridis.
PALETTE = list(mpl.colormaps["tab10"].colors)
SEQUENTIAL = "viridis"

SURFACE = "white"
INK = "black"
INK_SECONDARY = "dimgray"

STYLE = {
    "axes.titlesize": 11,
    "axes.labelsize": 10,
    "axes.spines.top": False,
    "axes.spines.right": False,
    "axes.grid": True,
    "axes.grid.axis": "y",
    "axes.axisbelow": True,
    "grid.linewidth": 0.8,
    "lines.linewidth": 2,
    "legend.frameon": False,
    "legend.fontsize": 9,
}

LABELS = {
    "best": "comprimento da melhor rota",
    "mean": "comprimento médio",
    "worst": "pior comprimento",
    "final_mean": "comprimento médio final",
    "gap_pct": "distância do ótimo (%)",
    "ratio_optimum": "comprimento / ótimo",
    "epochs": "épocas",
    "epoch_of_best": "época do melhor",
    "elapsed_ms": "tempo (ms)",
    "epoch": "época",
    "crossover": "crossover",
    "mutation": "mutação",
    "selection": "seleção",
    "populationSize": "população",
    "crossoverRate": "taxa de crossover",
    "mutationRate": "taxa de mutação",
    "tournamentSize": "tamanho do torneio",
    "truncationFraction": "fração do truncamento",
    "elitism": "elitismo",
    "n": "cidades",
    "config": "configuração",
}


AGG_LABELS = {"median": "mediana", "mean": "média", "min": "mínimo", "max": "máximo"}


def label(col: str) -> str:
    return LABELS.get(col, col)


@contextmanager
def style():
    with mpl.rc_context(STYLE):
        yield


def colors(values) -> dict:
    """Cor fixa por valor, em ordem crescente dos valores distintos."""
    levels = _levels(values)
    if len(levels) > len(PALETTE):
        raise ValueError(
            f"{len(levels)} séries; o máximo é {len(PALETTE)}. Divida com col=."
        )
    return dict(zip(levels, PALETTE))


def convergence(
    hist: pd.DataFrame,
    hue: str | None = None,
    col: str | None = None,
    metric: str = "best",
    logx: bool = False,
) -> plt.Figure:
    """Mediana entre as execuções a cada época, com faixa interquartil.

    Execuções que pararam antes (estagnação) mantêm o último valor até o fim,
    senão a mediana das épocas finais só contaria as execuções mais longas.
    """
    panels = [None] if col is None else _levels(hist[col])
    palette = colors(hist[hue]) if hue else {None: PALETTE[0]}
    with style():
        fig, axes = plt.subplots(
            1,
            len(panels),
            sharey=True,
            squeeze=False,
            figsize=(1.5 + 4 * len(panels), 3.6),
        )
        for ax, panel in zip(axes[0], panels):
            sub = hist if panel is None else hist[hist[col] == panel]
            for key, g in sub.groupby(hue) if hue else [(None, sub)]:
                wide = g.pivot_table(
                    index="epoch", columns=["experiment", "run"], values=metric
                ).ffill()
                if metric == "best":
                    wide = wide.cummin()
                q = wide.quantile([0.25, 0.5, 0.75], axis=1).T
                c = palette[key]
                ax.fill_between(
                    q.index, q[0.25], q[0.75], color=c, alpha=0.18, linewidth=0
                )
                ax.plot(
                    q.index, q[0.5], color=c, label=None if key is None else str(key)
                )
            if panel is not None:
                ax.set_title(f"{label(col)} = {panel}")
            ax.set_xlabel(label("epoch"))
            if logx:
                ax.set_xscale("log")
        axes[0][0].set_ylabel(f"{label(metric)} (mediana, IQR)")
        if hue:
            _legend(fig, axes[0][-1], hue)
        fig.tight_layout()
    return fig


def box(
    df: pd.DataFrame, x: str, y: str = "best", hue: str | None = None
) -> plt.Figure:
    """Distribuição do resultado final entre seeds, por configuração."""
    xs = _levels(df[x])
    hues = _levels(df[hue]) if hue else [None]
    palette = colors(df[hue]) if hue else {None: PALETTE[0]}
    width = 0.8 / len(hues)
    with style():
        fig, ax = plt.subplots(figsize=(1.5 + max(4, 0.9 * len(xs) * len(hues)), 3.8))
        for j, h in enumerate(hues):
            sub = df if h is None else df[df[hue] == h]
            data, positions = [], []
            for i, xv in enumerate(xs):
                values = sub.loc[sub[x] == xv, y].dropna()
                if len(values):
                    data.append(values)
                    positions.append(i - 0.4 + width * (j + 0.5))
            if not data:
                continue
            c = palette[h]
            ax.boxplot(
                data,
                positions=positions,
                widths=width * 0.8,
                patch_artist=True,
                boxprops={
                    "facecolor": mpl.colors.to_rgba(c, 0.35),
                    "edgecolor": c,
                    "linewidth": 1.2,
                },
                whiskerprops={"color": c, "linewidth": 1.2},
                capprops={"color": c, "linewidth": 1.2},
                medianprops={"color": INK, "linewidth": 1.6},
                flierprops={
                    "marker": "o",
                    "markersize": 3,
                    "markerfacecolor": c,
                    "markeredgecolor": "none",
                    "alpha": 0.6,
                },
            )
            if h is not None:
                ax.plot([], [], color=c, linewidth=6, alpha=0.6, label=str(h))
        names = [str(v) for v in xs]
        long = max(map(len, names)) > 10
        ax.set_xticks(
            range(len(xs)),
            names,
            rotation=30 if long else 0,
            ha="right" if long else "center",
        )
        ax.set_xlim(-0.6, len(xs) - 0.4)
        ax.set_xlabel(label(x))
        ax.set_ylabel(label(y))
        if hue:
            _legend(fig, ax, hue)
        fig.tight_layout()
    return fig


def heatmap(
    df: pd.DataFrame, rows: str, cols: str, value: str = "best", agg: str = "median"
) -> plt.Figure:
    """Interação entre dois fatores: `agg` de `value` em cada combinação."""
    table = df.pivot_table(index=rows, columns=cols, values=value, aggfunc=agg)
    table = table.reindex(index=_levels(df[rows]), columns=_levels(df[cols]))
    with style():
        fig, ax = plt.subplots(
            figsize=(2 + 1.1 * table.shape[1], 1.2 + 0.6 * table.shape[0])
        )
        im = ax.imshow(table.values, cmap=SEQUENTIAL, aspect="auto")
        ax.grid(False)
        ax.set_xticks(range(table.shape[1]), [str(c) for c in table.columns])
        ax.set_yticks(range(table.shape[0]), [str(r) for r in table.index])
        ax.set_xlabel(label(cols))
        ax.set_ylabel(label(rows))
        ax.spines[:].set_visible(False)
        ax.tick_params(length=0)
        lo, hi = np.nanmin(table.values), np.nanmax(table.values)
        for (i, j), v in np.ndenumerate(table.values):
            if np.isnan(v):
                continue
            dark = hi > lo and (v - lo) / (hi - lo) < 0.55
            ax.text(
                j,
                i,
                _fmt(v),
                ha="center",
                va="center",
                fontsize=9,
                color=SURFACE if dark else INK,
            )
        cbar = fig.colorbar(im, ax=ax, shrink=0.9)
        cbar.outline.set_visible(False)
        cbar.set_label(f"{label(value)} ({AGG_LABELS.get(agg, agg)})")
        fig.tight_layout()
    return fig


def sweep(
    df: pd.DataFrame,
    x: str,
    y: str = "best",
    hue: str | None = None,
    col: str | None = None,
    logx: bool = False,
    logy: bool = False,
) -> plt.Figure:
    """Parâmetro numérico no eixo x: mediana entre seeds com faixa interquartil."""
    panels = [None] if col is None else _levels(df[col])
    palette = colors(df[hue]) if hue else {None: PALETTE[0]}
    with style():
        fig, axes = plt.subplots(
            1,
            len(panels),
            sharey=True,
            squeeze=False,
            figsize=(1.5 + 4 * len(panels), 3.6),
        )
        for ax, panel in zip(axes[0], panels):
            sub = df if panel is None else df[df[col] == panel]
            for key, g in sub.groupby(hue) if hue else [(None, sub)]:
                q = g.groupby(x)[y].quantile([0.25, 0.5, 0.75]).unstack()
                c = palette[key]
                ax.fill_between(
                    q.index, q[0.25], q[0.75], color=c, alpha=0.18, linewidth=0
                )
                ax.plot(
                    q.index,
                    q[0.5],
                    color=c,
                    marker="o",
                    markersize=6,
                    markeredgecolor=SURFACE,
                    markeredgewidth=1.5,
                    label=None if key is None else str(key),
                )
            if logx:
                ax.set_xscale("log")
            if logy:
                ax.set_yscale("log")
            # Marcas só nos valores testados.
            levels = _levels(sub[x])
            ax.set_xticks(levels, [f"{v:g}" for v in levels])
            ax.minorticks_off()
            if panel is not None:
                ax.set_title(f"{label(col)} = {panel}")
            ax.set_xlabel(label(x))
        axes[0][0].set_ylabel(f"{label(y)} (mediana, IQR)")
        if hue:
            _legend(fig, axes[0][-1], hue)
        fig.tight_layout()
    return fig


def route(
    run: Path | Run, epoch: int | None = None, ax: plt.Axes | None = None
) -> plt.Figure:
    """Melhor rota gravada até `epoch` (padrão: a última)."""
    pts = load.points(run)
    rts = load.routes(run)
    if epoch is not None:
        rts = rts[rts["epoch"] <= epoch]
    row = rts.iloc[-1]
    order = row["route"] + row["route"][:1]
    xy = pts.loc[order, ["x", "y"]].to_numpy()
    length = np.hypot(*np.diff(xy, axis=0).T).sum()

    with style():
        if ax is None:
            fig, ax = plt.subplots(figsize=(4.5, 4.5))
        else:
            fig = ax.figure
        ax.plot(xy[:, 0], xy[:, 1], color=PALETTE[0], linewidth=1.5, zorder=1)
        ax.scatter(
            pts["x"],
            pts["y"],
            s=14,
            color=INK_SECONDARY,
            edgecolor=SURFACE,
            linewidth=0.8,
            zorder=2,
        )
        ax.set_aspect("equal")
        ax.grid(False)
        ax.set_xticks([])
        ax.set_yticks([])
        ax.spines[:].set_visible(False)
        ax.set_title(f"época {row['epoch']}: {length:.1f}", fontsize=10)
    return fig


def route_evolution(run: Path | Run, panels: int = 4) -> plt.Figure:
    """Melhor rota em `panels` momentos gravados, do início ao fim."""
    epochs = load.routes(run)["epoch"].to_numpy()
    picks = epochs[
        np.unique(np.linspace(0, len(epochs) - 1, panels).round().astype(int))
    ]
    with style():
        fig, axes = plt.subplots(1, len(picks), figsize=(3.2 * len(picks), 3.4))
        for ax, e in zip(np.atleast_1d(axes), picks):
            route(run, epoch=int(e), ax=ax)
        fig.tight_layout()
    return fig


def save(
    fig: plt.Figure, exp: str | Path | Experiment, name: str, dpi: int = 200
) -> Path:
    exp = load_experiment(exp)
    path = exp.figures_dir / f"{name}.png"
    path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(path, dpi=dpi, bbox_inches="tight")
    plt.close(fig)
    return path


def default_figures(exp: str | Path | Experiment) -> list[Path]:
    """Gráficos padrão a partir dos fatores que variam no experimento.

    Com "problems" (cenário, n), cada problema ganha sua pasta em figures/
    (ex.: figures/uniform_n500/) e scaling_<cenário>.png mostra o resultado por n.
    """
    exp = load_experiment(exp)
    df = load.summary(exp)
    if "best" not in df or df["best"].isna().all():
        print(f"{exp.name}: stats.csv vazios, nada para plotar")
        return []

    problem = [c for c in load.problem_factors(exp) if c in df and df[c].nunique() > 1]
    factors = [f for f in load.factors(exp, df) if f not in problem]
    if isinstance(exp.definition.get("grid"), list):
        # Grids unidos não se cruzam (ex.: tamanho do torneio só existe no torneio):
        # cada combinação vira um nível de um fator só.
        factors = ["config"]
    f0 = factors[0] if factors else None
    f1 = factors[1] if len(factors) > 1 else None

    hist = load.history(exp)
    saved = []
    for key, sub in df.groupby(problem) if problem else [((), df)]:
        key = key if isinstance(key, tuple) else (key,)
        folder = "_".join(
            str(v) if c == "scenario" else f"{c}{v}" for c, v in zip(problem, key)
        )
        prefix = f"{folder}/" if folder else ""
        saved += _problem_figures(
            exp, sub, hist[hist["run"].isin(sub["run"])], f0, f1, prefix
        )

    if "n" in problem and f0:
        scenarios = _levels(df["scenario"]) if "scenario" in df else [None]
        for scen in scenarios:
            sub = df if scen is None else df[df["scenario"] == scen]
            y = "best"
            if scen == "circle":
                # Razão em vez de gap_pct: no ótimo o gap é ~0, que some no eixo log.
                sub = sub.assign(ratio_optimum=sub["best"] / sub["optimum"])
                y = "ratio_optimum"
            fig = sweep(sub, x="n", y=y, hue=f0, col=f1, logx=True, logy=True)
            saved.append(save(fig, exp, f"scaling_{scen}" if scen else "scaling"))

    best_run = exp.dir / "runs" / df.loc[df["best"].idxmin(), "run"]
    if not load.routes(best_run).empty:  # vazio com --save-routes 0
        saved.append(save(route_evolution(best_run), exp, "best_route"))
    return saved


def _problem_figures(
    exp: Experiment,
    df: pd.DataFrame,
    hist: pd.DataFrame,
    f0: str | None,
    f1: str | None,
    prefix: str,
) -> list[Path]:
    """Convergência, distribuição e interação dos fatores em um problema só."""
    saved = []
    col = f1 if f1 and df[f1].nunique() <= 4 else None
    fig = convergence(hist, hue=f0, col=col)
    saved.append(save(fig, exp, f"{prefix}convergence"))
    if f0:
        if pd.api.types.is_numeric_dtype(df[f0]):
            values = df[f0][df[f0] > 0]
            logx = len(values) == len(df) and values.max() / values.min() >= 20
            fig = sweep(df, x=f0, hue=f1, logx=logx)
            saved.append(save(fig, exp, f"{prefix}sweep"))
        else:
            saved.append(save(box(df, x=f0, hue=f1), exp, f"{prefix}box"))
    if f1:
        fig = heatmap(df, rows=f0, cols=f1)
        saved.append(save(fig, exp, f"{prefix}heatmap"))
    if "gap_pct" in df and df["gap_pct"].notna().any():
        fig = box(df, x=f0 or "n", y="gap_pct", hue=f1)
        saved.append(save(fig, exp, f"{prefix}gap"))
    return saved


def _levels(values) -> list:
    """Valores distintos em ordem natural (números como números)."""
    return sorted(
        pd.unique(pd.Series(values).dropna()), key=lambda v: (isinstance(v, str), v)
    )


def _legend(fig: plt.Figure, ax: plt.Axes, hue: str) -> None:
    ax.legend(
        title=label(hue), loc="upper left", bbox_to_anchor=(1.01, 1), borderaxespad=0
    )


def _fmt(v: float) -> str:
    # Sem separador de milhar: "7,220" pareceria decimal em português.
    return f"{v:.0f}" if abs(v) >= 100 else f"{v:.2f}"
