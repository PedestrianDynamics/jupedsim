import argparse
import pathlib
import shutil
import logging
import docker
import tarfile

logging.basicConfig(
    level=logging.DEBUG,
    format="%(asctime)s - %(levelname)s - %(message)s",
)

perf_container_file = pathlib.Path("../container/perf-measurement/Dockerfile")
perf_container_tag = "perf-test"


def parse_args():
    parser = argparse.ArgumentParser('JuPedSim Performance Tests')
    parser.add_argument("-o", "--output", help="output directory", type=pathlib.Path)
    parser.add_argument("-s", "--source", help="source directory of JuPedSim", type=pathlib.Path)

    known, forwarded_args = parser.parse_known_args()
    if forwarded_args and forwarded_args[0] != "--":
        logging.warning(f"found unknown arguments: '{' '.join(forwarded_args)}' will be ignored. If you want to pass "
                        f"them to the container, separate them with '--', e.g., \n"
                        f"$ python run_perf_test.py -- -t large_street_network -- --limit 10000")

    else:
        forwarded_args = forwarded_args[1:]

    return known, forwarded_args


def build_docker_container(client, source_dir: pathlib.Path):
    perf_test_image, build_logs = client.images.build(
        path=source_dir.absolute().__str__(),
        dockerfile=perf_container_file.absolute().__str__(),
        tag=perf_container_tag,
    )
    for chunk in build_logs:
        if 'stream' in chunk:
            for line in chunk['stream'].splitlines():
                logging.info(line)

    return perf_test_image


def get_results(container, output_dir: pathlib.Path):
    with open("results.tar", "wb") as f:
        bits, stat = container.get_archive("/build/results/")
        for chunk in bits:
            f.write(chunk)

    tar_file = pathlib.Path("results.tar")
    if tarfile.is_tarfile(tar_file):
        with tarfile.open(tar_file.absolute()) as f:
            f.extractall(path=output_dir)
    tar_file.unlink()

    # move all files from results folder to
    for file in (output_dir / "results").iterdir():
        shutil.move(file, output_dir / file.name)
    (output_dir / "results").rmdir()


def main():
    parsed_args, forwarded_args = parse_args()

    client = docker.from_env()
    perf_test_image = build_docker_container(client, parsed_args.source)

    container = client.containers.run(
        perf_test_image.tags[0],
        privileged=True,
        cap_add=["CAP_PERFMON"],
        volumes={
            str(pathlib.Path(parsed_args.source).absolute()): {"bind": "/src", "mode": "ro"},
        },
        detach=True,
        name="perf-container-python",
        command=[*forwarded_args],
    )

    # print the output from the container
    output = container.attach(stdout=True, stream=True, logs=True)
    for line in output:
        print(line.decode('utf-8').strip())

    get_results(container, parsed_args.output)
    
    container.remove()


if __name__ == "__main__":
    main()
