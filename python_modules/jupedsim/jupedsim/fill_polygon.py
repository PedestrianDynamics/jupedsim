import distributions
import argparse


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('a_r', type=float, help="first argument: minimal radius among agents")
    parser.add_argument('w_d', type=float, help="second argument: minimal radius between agents and polygon segments")
    parser.add_argument('-agents', type=int, help="number of agents placed, default value=10", default=10)
    parser.add_argument('-point', '-p', type=str, nargs="+",
                        help="define the Polygon with it´s corner Points, formatted like '(0,0)' '(0,1)'")
    parser.add_argument('-a', action="store_true",
                        help="-a for as many agents as possible with bridson´s poisson-disk algorithm", dest="all")
    parser.add_argument('-heatmap', type=int, default=None,
                        help="creates a heatmap regarding as many iterations as selected")
    args = parser.parse_args()
    polygon = distributions.make_polygon(args.point)
    samples = None
    if args.heatmap is not None:
        distributions.heatmap(args.all, args.a_r, args.w_d, polygon, args.heatmap, args.agents)
        exit(0)
    elif args.all:
        samples = distributions.create_points_everywhere(polygon, args.a_r, args.w_d)
    else:
        samples = distributions.create_random_points(polygon, args.agents, args.a_r, args.w_d)

    distributions.show_points(polygon, samples, args.a_r)


if __name__ == "__main__":
    main()

