"""Testes de caixa-preta do executável ga (rode `cmake --build build` antes).

Quando o solver existir, estender para stats.csv/routes.csv, ignorando a
coluna elapsed_ms, que varia entre execuções.
"""

import csv
import json
import math
import subprocess
from pathlib import Path

import pytest

GA = Path(__file__).resolve().parents[1] / "build" / "ga"

pytestmark = pytest.mark.skipif(not GA.exists(), reason="build/ga não compilado")


def run_ga(out: Path, *args: str) -> Path:
    subprocess.run([str(GA), "--out", str(out), *args], check=True, capture_output=True)
    return out


def read_points(run: Path) -> list[tuple[float, float]]:
    with open(run / "points.csv") as f:
        return [(float(r["x"]), float(r["y"])) for r in csv.DictReader(f)]


def test_same_seeds_same_files(tmp_path):
    a = run_ga(tmp_path / "a", "--instance-seed", "7", "--seed", "3")
    b = run_ga(tmp_path / "b", "--instance-seed", "7", "--seed", "3")
    assert (a / "points.csv").read_bytes() == (b / "points.csv").read_bytes()


def test_instance_seed_changes_points(tmp_path):
    a = run_ga(tmp_path / "a", "--instance-seed", "1")
    b = run_ga(tmp_path / "b", "--instance-seed", "2")
    assert read_points(a) != read_points(b)


def test_ga_seed_does_not_change_points(tmp_path):
    a = run_ga(tmp_path / "a", "--seed", "1")
    b = run_ga(tmp_path / "b", "--seed", "2")
    assert read_points(a) == read_points(b)


def test_uniform_points_inside_square(tmp_path):
    run = run_ga(tmp_path / "u", "--n", "200", "--side", "100")
    points = read_points(run)
    assert len(points) == 200
    assert all(0 <= x < 100 and 0 <= y < 100 for x, y in points)


def test_circle_points_on_radius_and_shuffled(tmp_path):
    r = 500.0
    run = run_ga(tmp_path / "c", "--scenario", "circle", "--n", "50", "--radius", str(r))
    points = read_points(run)
    assert all(math.isclose(math.hypot(x - r, y - r), r, rel_tol=1e-6) for x, y in points)
    # Sem embaralhar, os ângulos estariam em ordem crescente pelo índice.
    angles = [math.atan2(y - r, x - r) % (2 * math.pi) for x, y in points]
    assert angles != sorted(angles)


def test_config_json_records_seeds(tmp_path):
    run = run_ga(tmp_path / "s", "--instance-seed", "5", "--seed", "9")
    cfg = json.loads((run / "config.json").read_text())
    assert cfg["instanceSeed"] == 5
    assert cfg["seed"] == 9


def test_too_few_cities_fails(tmp_path):
    result = subprocess.run([str(GA), "--n", "7", "--out", str(tmp_path / "x")],
                            capture_output=True, text=True)
    assert result.returncode != 0
    assert "8 cidades" in result.stderr
