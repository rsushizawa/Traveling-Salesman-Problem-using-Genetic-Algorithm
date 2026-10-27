import sys
from pathlib import Path

import matplotlib
import pytest

matplotlib.use("Agg")

from tsp import compare, experiment, load, plots  # noqa: E402

ROOT = Path(__file__).resolve().parents[1]
FAKE_GA = [sys.executable, str(ROOT / "tests" / "fake_ga.py")]

pytestmark = pytest.mark.skipif(
    not experiment.GA.exists(), reason="build/ga não compilado"
)

BASE = {"n": 20, "max-epochs": 60, "stagnation": 0}
GRID = {"mutation": ["swap", "insertion", "inversion"], "mut": [0.05, 0.2]}


@pytest.fixture
def exp(tmp_path):
    return experiment.run(
        tmp_path / "exp",
        base=BASE,
        grid=GRID,
        seeds=3,
        instance_seeds=[1, 2],
        ga=FAKE_GA,
        quiet=True,
    )


def test_plan_is_cartesian_product_times_seeds(exp):
    plan = exp.plan()
    assert len(plan) == 3 * 2 * 3 * 2
    assert len({r.id for r in plan}) == len(plan)
    assert plan[0].id == "mutation-swap_mut-0.05_i1_s0"


def test_rerun_skips_finished_runs(exp):
    stamp = {r.id: (r.dir / "stats.csv").stat().st_mtime_ns for r in exp.plan()}
    experiment.run(exp.dir, ga=FAKE_GA, quiet=True)
    assert stamp == {r.id: (r.dir / "stats.csv").stat().st_mtime_ns for r in exp.plan()}


def test_changing_grid_runs_only_new_configs(exp):
    old = {r.id: (r.dir / "stats.csv").stat().st_mtime_ns for r in exp.plan()}
    grid = {**GRID, "mutation": [*GRID["mutation"], "scramble"]}
    exp2 = experiment.run(
        exp.dir,
        base=BASE,
        grid=grid,
        seeds=3,
        instance_seeds=[1, 2],
        ga=FAKE_GA,
        quiet=True,
    )
    new = {r.id: (r.dir / "stats.csv").stat().st_mtime_ns for r in exp2.plan()}
    assert all(new[k] == v for k, v in old.items())
    assert len(load.summary(exp2)) == 4 * 2 * 3 * 2


def test_reserved_flags_rejected(tmp_path):
    with pytest.raises(ValueError, match="seed"):
        experiment.define(tmp_path / "x", base={"seed": 3})


def test_failures_are_reported(tmp_path):
    with pytest.raises(RuntimeError, match="8 cidades"):
        experiment.run(tmp_path / "bad", base={"n": 5}, seeds=1, ga=FAKE_GA, quiet=True)


def test_summary_has_config_and_metrics(exp):
    df = load.summary(exp)
    assert len(df) == 36
    assert load.factors(exp) == ["mutation", "mutationRate"]
    assert list(df.columns[2:5]) == ["config", "mutation", "mutationRate"]
    assert df["config"].nunique() == 6
    assert {
        "best",
        "epochs",
        "epoch_of_best",
        "elapsed_ms",
        "seed",
        "instanceSeed",
    } <= set(df)
    assert (df["epochs"] == 60).all()


def test_circle_summary_has_gap(tmp_path):
    exp = experiment.run(
        tmp_path / "c",
        base={**BASE, "scenario": "circle"},
        seeds=2,
        ga=FAKE_GA,
        quiet=True,
    )
    df = load.summary(exp)
    assert (df["gap_pct"] > 0).all()


def test_history_is_long_format(exp):
    hist = load.history(exp)
    assert len(hist) == 36 * 60
    assert {"epoch", "best", "mutation", "mutationRate", "run"} <= set(hist)


def test_compare(exp):
    df = load.summary(exp)
    t = compare.table(df)
    assert len(t) == 6 and t["runs"].eq(6).all()
    w = compare.wilcoxon(df, "mutation", "inversion", "swap")
    assert w["n"] == 12  # pareado por (mutationRate, instanceSeed, seed)
    f = compare.friedman(df, "mutation")
    assert f["n_blocks"] == 12 and len(f["mean_rank"]) == 3
    assert len(compare.pairwise(df, "mutation")) == 3
    with pytest.raises(ValueError, match="wilcoxon"):
        compare.friedman(df, "mutationRate")


def test_default_figures(exp):
    paths = plots.default_figures(exp)
    assert {p.name for p in paths} == {
        "convergence.png",
        "box.png",
        "heatmap.png",
        "best_route.png",
    }
    assert all(p.stat().st_size > 0 for p in paths)


def test_union_of_grids_uses_config_as_factor(tmp_path):
    grid = [{"mutation": ["swap"], "mut": [0.05, 0.2]}, {"mutation": ["inversion"]}]
    exp = experiment.run(
        tmp_path / "u", base=BASE, grid=grid, seeds=3, ga=FAKE_GA, quiet=True
    )
    df = load.summary(exp)
    assert sorted(df["config"].unique()) == [
        "mutation-inversion",
        "mutation-swap_mut-0.05",
        "mutation-swap_mut-0.2",
    ]
    names = {p.name for p in plots.default_figures(exp)}
    assert names == {"convergence.png", "box.png", "best_route.png"}


def test_problems_cross_grid_without_changing_config(tmp_path):
    problems = {"scenario": ["uniform", "circle"], "n": [12, 20]}
    exp = experiment.run(
        tmp_path / "p",
        base=BASE,
        grid={"mutation": ["swap", "inversion"]},
        problems=problems,
        seeds=2,
        ga=FAKE_GA,
        quiet=True,
    )
    plan = exp.plan()
    assert len(plan) == 2 * 4 * 2
    assert plan[0].id == "mutation-swap_scenario-uniform_n-12_i1_s0"
    df = load.summary(exp)
    assert sorted(df["config"].unique()) == ["mutation-inversion", "mutation-swap"]
    assert load.factors(exp) == ["mutation", "scenario", "n"]
    figures = plots.default_figures(exp)
    names = {p.relative_to(exp.figures_dir).as_posix() for p in figures}
    assert {
        "uniform_n12/convergence.png",
        "circle_n20/gap.png",
        "scaling_uniform.png",
        "scaling_circle.png",
    } <= names


def test_shards_partition_the_plan(exp):
    plan = exp.plan()
    parts = [experiment.split(plan, i, 5) for i in range(5)]
    ids = [r.id for part in parts for r in part]
    assert sorted(ids) == sorted(r.id for r in plan)
    assert all(parts)
    with pytest.raises(ValueError):
        experiment.split(plan, 5, 5)
