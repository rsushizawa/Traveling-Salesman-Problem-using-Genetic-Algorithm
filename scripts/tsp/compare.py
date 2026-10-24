from __future__ import annotations

import itertools

import pandas as pd

from .load import BLOCKS, varied_columns


def table(
    df: pd.DataFrame, by: list[str] | str | None = None, metric: str = "best"
) -> pd.DataFrame:
    """Resumo por configuração, ordenado pela mediana (menor é melhor)."""
    by = by or varied_columns(df)
    g = df.groupby(by)
    out = g[metric].agg(
        median="median",
        q1=lambda s: s.quantile(0.25),
        q3=lambda s: s.quantile(0.75),
        mean="mean",
        std="std",
        min="min",
    )
    out["runs"] = g.size()
    if "gap_pct" in df and df["gap_pct"].notna().any():
        out["gap_pct_median"] = g["gap_pct"].median()
    out["epochs_mean"] = g["epochs"].mean()
    out["elapsed_ms_mean"] = g["elapsed_ms"].mean()
    return out.sort_values("median")


def paired(df: pd.DataFrame, factor: str, metric: str = "best") -> pd.DataFrame:
    """Uma coluna por nível de `factor`, uma linha por bloco (só blocos completos)."""
    blocks = [c for c in varied_columns(df) if c != factor] + BLOCKS
    wide = df.pivot_table(index=blocks, columns=factor, values=metric)
    return wide.dropna()


def wilcoxon(df: pd.DataFrame, factor: str, a, b, metric: str = "best") -> dict:
    """`a` contra `b` em `factor`, pareado por bloco."""
    from scipy.stats import wilcoxon as test

    wide = paired(df, factor, metric)
    diff = wide[a] - wide[b]
    result = test(wide[a], wide[b])
    return {
        "a": a,
        "b": b,
        "n": len(wide),
        "median_a": wide[a].median(),
        "median_b": wide[b].median(),
        "median_diff": diff.median(),
        "a_wins": int((diff < 0).sum()),
        "b_wins": int((diff > 0).sum()),
        "p_value": result.pvalue,
    }


def friedman(df: pd.DataFrame, factor: str, metric: str = "best") -> dict:
    """Há diferença entre os níveis de `factor`? Inclui o rank médio (1 = melhor)."""
    from scipy.stats import friedmanchisquare

    wide = paired(df, factor, metric)
    if wide.shape[1] < 3:
        raise ValueError(f"{factor} tem {wide.shape[1]} níveis; com 2 use wilcoxon()")
    result = friedmanchisquare(*(wide[c] for c in wide.columns))
    return {
        "n_blocks": len(wide),
        "statistic": result.statistic,
        "p_value": result.pvalue,
        "mean_rank": wide.rank(axis=1).mean().sort_values(),
    }


def pairwise(df: pd.DataFrame, factor: str, metric: str = "best") -> pd.DataFrame:
    """Wilcoxon entre todos os pares de níveis, com p corrigido por Holm."""
    levels = list(paired(df, factor, metric).columns)
    rows = [
        wilcoxon(df, factor, a, b, metric) for a, b in itertools.combinations(levels, 2)
    ]
    out = pd.DataFrame(rows)
    out["p_holm"] = _holm(out["p_value"])
    return out.sort_values("p_holm")


def _holm(p: pd.Series) -> pd.Series:
    order = p.sort_values().index
    m = len(p)
    adjusted, running = {}, 0.0
    for k, i in enumerate(order):
        running = max(running, min(1.0, (m - k) * p[i]))
        adjusted[i] = running
    return pd.Series(adjusted).reindex(p.index)
