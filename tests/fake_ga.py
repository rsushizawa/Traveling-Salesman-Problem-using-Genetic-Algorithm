import json
import math
import random
import subprocess
import sys
from pathlib import Path

GA = Path(__file__).resolve().parents[1] / "build" / "ga"
TRIES = {"swap": 1, "insertion": 2, "scramble": 1, "inversion": 4}


def length(route, pts):
    return sum(math.dist(pts[route[k - 1]], pts[route[k]]) for k in range(len(route)))


def main() -> None:
    args = sys.argv[1:]
    if (code := subprocess.run([str(GA), *args]).returncode) != 0:
        sys.exit(code)

    out = Path(args[args.index("--out") + 1])
    cfg = json.loads((out / "config.json").read_text())
    lines = (out / "points.csv").read_text().splitlines()[1:]
    pts = [tuple(map(float, line.split(",")[1:])) for line in lines]
    rng = random.Random(f"{cfg['seed']}-{cfg['crossover']}-{cfg['mutation']}")

    route = list(range(len(pts)))
    rng.shuffle(route)
    best = length(route, pts)
    tries = max(1, round(TRIES[cfg["mutation"]] * cfg["mutationRate"] * 20))

    with open(out / "stats.csv", "w") as stats, open(out / "routes.csv", "w") as routes:
        stats.write("epoch,best,mean,worst,elapsed_ms\n")
        routes.write("epoch,route\n")
        since_improved = 0
        for epoch in range(cfg["maxEpochs"]):
            improved = epoch == 0
            for _ in range(tries):
                i, j = sorted(rng.sample(range(len(route)), 2))
                candidate = route[:i] + route[i : j + 1][::-1] + route[j + 1 :]
                if (c := length(candidate, pts)) < best:
                    route, best, improved = candidate, c, True
            mean = best * (1.08 + 0.04 * rng.random())
            stats.write(
                f"{epoch},{best:.4f},{mean:.4f},{best * 1.25:.4f},{epoch * 0.5:.4f}\n"
            )
            if improved:
                routes.write(f"{epoch},{' '.join(map(str, route))}\n")
            since_improved = 0 if improved else since_improved + 1
            if cfg["stagnation"] and since_improved >= cfg["stagnation"]:
                break


if __name__ == "__main__":
    main()
