#!/usr/bin/env python3

import argparse

from tsp import experiment, plots


def main() -> None:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument("experiments", nargs="+", help="pasta ou nome em experiments/")
    parser.add_argument(
        "--jobs", type=int, help="execuções em paralelo (padrão: nº de CPUs)"
    )
    parser.add_argument("--ga", default=experiment.GA, help="executável do solver")
    parser.add_argument(
        "--force", action="store_true", help="refaz execuções já concluídas"
    )
    parser.add_argument("--no-plots", action="store_true", help="não gera os gráficos")
    args = parser.parse_args()

    for path in args.experiments:
        exp = experiment.run(path, jobs=args.jobs, ga=args.ga, force=args.force)
        if not args.no_plots:
            for fig in plots.default_figures(exp):
                print(fig)


if __name__ == "__main__":
    main()
