import argparse
import pathlib
import sys
import logging
import docker

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s - %(levelname)s - %(message)s",
)

perf_container_file = pathlib.Path("../container/perf-measurement/Dockerfile")
perf_container_tag = "perf-test"


def parse_args():
    parser = argparse.ArgumentParser('JuPedSim Performance Tests')
    parser.add_argument("-o", "--output", help="output directory", type=pathlib.Path)
    parser.add_argument("-t", "--test", choices=["grosser_stern", "large_street_network", "all"], default="all")
    parser.add_argument("-s", "--source")

    seperator_index = sys.argv.index("--") if "--" in sys.argv else len(sys.argv)
    to_parse = sys.argv[1:seperator_index]
    known, unknown = parser.parse_known_args(to_parse)
    if unknown:
        logging.warning(f"found unknown arguments: '{' '.join(unknown)}' will be ignored. If you want to pass them to "
                        f"the tests, separate them with '--', e.g., \n"
                        f"$ python run_perf_test.py -t large_street_network -- --limit 10000")

    remainder = sys.argv[seperator_index + 1:]
    if known.test == "all" and remainder:
        logging.warning(f"found arguments to be forwarded to test: '{' '.join(remainder)}', but no test specified. "
                        f"Arguments will be ignored.")
    return known, remainder


def build_docker_container(client):
    # TODO context: .. or better as input?
    perf_test_image, logs = client.images.build(
        path="..",
        dockerfile=perf_container_file.absolute().__str__(),
        tag=perf_container_tag,
    )
    print(list(logs))
    return perf_test_image

    # perf_test_image = docker.legacy_build(
    #     "..",
    #     tags=perf_container_tag,
    #     file=perf_container_file.absolute().__str__()
    # )
    # return perf_test_image


def build_jupedsim(container):
    _, log = container.exec_run(
        ["bash", "/src/scripts/perf-test/build_jupedsim.sh"],
        stream=True,
    )
    for line in log:
        print(line.decode('utf-8').strip())


def run_perf_tests(container, test_selection: str):
    if test_selection in ["all", "large_street_network"]:
        print("large_street_network")
        _, log = container.exec_run(
            ["bash", "/src/scripts/perf-test/perf-large-street-network.sh"],
            stream=True,
            privileged=True,
        )
        for line in log:
            print(line.decode(), end='')

        with open("large_street_network.svg", 'wb') as f:
            bits, stat = container.get_archive("/build/large_street_network_perf.svg")
            for chunk in bits:
                f.write(chunk)

    if test_selection in ["all", "grosser_stern"]:
        print("grosser stern")
        _, log = container.exec_run(
            ["bash", "/src/scripts/perf-test/perf-grosser-stern.sh"],
            stream=True,
            privileged=True,
        )

        with open("grosser_stern.svg", 'wb') as f:
            bits, stat = container.get_archive("/build/grosser_stern.svg")
            for chunk in bits:
                f.write(chunk)

        # for line in log:
        #     print(line.decode().strip())



def main():
    parsed_args, forwarded_args = parse_args()

    client = docker.from_env()
    perf_test_image = build_docker_container(client)

    container = client.containers.run(
        perf_test_image.tags[0],
        privileged=True,
        volumes={
            "/home/tobias/development/jupedsim/jpscore": {"bind": "/src", "mode": "ro"},
        },
        detach=True,
        name="perf-container-python",
        # entrypoint=['bash', '/src/scripts/perf-test/build_jupedsim.sh'],
        entrypoint=['bash'],
        # entrypoint=['bash', "while :; do :; done & kill -STOP $! && wait $!"],
        # tty=True,
        # auto_remove=True,
        stdin_open=True, tty=True
    )

    build_jupedsim(container)
    run_perf_tests(container, parsed_args.test)

    container.stop()
    container.remove()


if __name__ == "__main__":
    main()
