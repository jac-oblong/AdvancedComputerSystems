#!/usr/bin/env python

import subprocess
import os


def run_mlc(granularity, fp):
    result = subprocess.run(
        ["mlc", "--max_bandwidth", f"-l{granularity}"],
        encoding="utf-8",
        capture_output=True,
    )
    # find the bandwidth
    for line in result.stdout.split("\n")[4:]:
        if ":" not in line:
            continue
        ratio = line[: line.rfind(":")]
        bandwidth = line[line.rfind(":") + 1 :]
        fp.write(f"{granularity}, {ratio}, {bandwidth}\n")


try:
    os.remove("log.txt")
except FileNotFoundError:
    pass


with open("log.txt", "w", encoding="utf-8") as fp:
    granularities = [64, 256, 1024]
    for granularity in granularities:
        run_mlc(granularity, fp)
