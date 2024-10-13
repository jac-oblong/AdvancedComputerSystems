#!/usr/bin/env python

import subprocess
import os

# fmt: off
LOG_FILE = "log.txt"
# Matrix Size, Sparcity 1, Sparcity 2, Sparse Compression, Threads, SIMD, Cache
COMMANDS = {
    # different sizes with sparcity 0
    "500, 0, 0, 0, 1, 0, 0": ["./main", "500"],
    "1000, 0, 0, 0, 1, 0, 0": ["./main", "1000"],
    "2500, 0, 0, 0, 1, 0, 0": ["./main", "2500"],
    "5000, 0, 0, 0, 1, 0, 0": ["./main", "5000"],
    # different sizes with sparcity 0.1
    "500, 0.1, 0.1, 0, 1, 0, 0": ["./main", "500", "-s1", "0.1", "-s2", "0.1"],
    "1000, 0.1, 0.1, 0, 1, 0, 0": ["./main", "1000", "-s1", "0.1", "-s2", "0.1"],
    "2500, 0.1, 0.1, 0, 1, 0, 0": ["./main", "2500", "-s1", "0.1", "-s2", "0.1"],
    "5000, 0.1, 0.1, 0, 1, 0, 0": ["./main", "5000", "-s1", "0.1", "-s2", "0.1"],
    # different sizes with sparcity 1
    "500, 1, 1, 0, 1, 0, 0": ["./main", "500", "-s1", "1", "-s2", "1"],
    "1000, 1, 1, 0, 1, 0, 0": ["./main", "1000", "-s1", "1", "-s2", "1"],
    "2500, 1, 1, 0, 1, 0, 0": ["./main", "2500", "-s1", "1", "-s2", "1"],
    "5000, 1, 1, 0, 1, 0, 0": ["./main", "5000", "-s1", "1", "-s2", "1"],
    # different sizes with sparcity 5
    "500, 5, 5, 0, 1, 0, 0": ["./main", "500", "-s1", "5", "-s2", "5"],
    "1000, 5, 5, 0, 1, 0, 0": ["./main", "1000", "-s1", "5", "-s2", "5"],
    "2500, 5, 5, 0, 1, 0, 0": ["./main", "2500", "-s1", "5", "-s2", "5"],
    "5000, 5, 5, 0, 1, 0, 0": ["./main", "5000", "-s1", "5", "-s2", "5"],
    # different sizes with sparcity 0 and threading
    "500, 0, 0, 0, 4, 0, 0": ["./main", "500", "-t", "4"],
    "1000, 0, 0, 0, 4, 0, 0": ["./main", "1000", "-t", "4"],
    "2500, 0, 0, 0, 4, 0, 0": ["./main", "2500", "-t", "4"],
    "5000, 0, 0, 0, 4, 0, 0": ["./main", "5000", "-t", "4"],
    # different sizes with sparcity 1 and threading
    "500, 1, 1, 0, 4, 0, 0": ["./main", "500", "-s1", "1", "-s2", "1", "-t", "4"],
    "1000, 1, 1, 0, 4, 0, 0": ["./main", "1000", "-s1", "1", "-s2", "1", "-t", "4"],
    "2500, 1, 1, 0, 4, 0, 0": ["./main", "2500", "-s1", "1", "-s2", "1", "-t", "4"],
    "5000, 1, 1, 0, 4, 0, 0": ["./main", "5000", "-s1", "1", "-s2", "1", "-t", "4"],
    # different sizes with sparcity 0 and simd
    "500, 0, 0, 0, 1, 1, 0": ["./main", "500", "-s"],
    "1000, 0, 0, 0, 1, 1, 0": ["./main", "1000", "-s"],
    "2500, 0, 0, 0, 1, 1, 0": ["./main", "2500", "-s"],
    "5000, 0, 0, 0, 1, 1, 0": ["./main", "5000", "-s"],
    # different sizes with sparcity 1 and simd
    "500, 1, 1, 0, 1, 1, 0": ["./main", "500", "-s1", "1", "-s2", "1", "-s"],
    "1000, 1, 1, 0, 1, 1, 0": ["./main", "1000", "-s1", "1", "-s2", "1", "-s"],
    "2500, 1, 1, 0, 1, 1, 0": ["./main", "2500", "-s1", "1", "-s2", "1", "-s"],
    "5000, 1, 1, 0, 1, 1, 0": ["./main", "5000", "-s1", "1", "-s2", "1", "-s"],
    # different sizes with sparcity 0 and cache
    "500, 0, 0, 0, 1, 0, 1": ["./main", "500", "-c"],
    "1000, 0, 0, 0, 1, 0, 1": ["./main", "1000", "-c"],
    "2500, 0, 0, 0, 1, 0, 1": ["./main", "2500", "-c"],
    "5000, 0, 0, 0, 1, 0, 1": ["./main", "5000", "-c"],
    # different sizes with sparcity 1 and cache
    "500, 1, 1, 0, 1, 0, 1": ["./main", "500", "-s1", "1", "-s2", "1", "-c"],
    "1000, 1, 1, 0, 1, 0, 1": ["./main", "1000", "-s1", "1", "-s2", "1", "-c"],
    "2500, 1, 1, 0, 1, 0, 1": ["./main", "2500", "-s1", "1", "-s2", "1", "-c"],
    "5000, 1, 1, 0, 1, 0, 1": ["./main", "5000", "-s1", "1", "-s2", "1", "-c"],
    # different sizes with sparcity 0 and threading and cache
    "500, 0, 0, 0, 4, 0, 1": ["./main", "500", "-t", "4", "-c"],
    "1000, 0, 0, 0, 4, 0, 1": ["./main", "1000", "-t", "4", "-c"],
    "2500, 0, 0, 0, 4, 0, 1": ["./main", "2500", "-t", "4", "-c"],
    "5000, 0, 0, 0, 4, 0, 1": ["./main", "5000", "-t", "4", "-c"],
    # different sizes with sparcity 1 and threading and cache
    "500, 1, 1, 0, 4, 0, 1": ["./main", "500", "-s1", "1", "-s2", "1", "-t", "4", "-c"],
    "1000, 1, 1, 0, 4, 0, 1": ["./main", "1000", "-s1", "1", "-s2", "1", "-t", "4", "-c"],
    "2500, 1, 1, 0, 4, 0, 1": ["./main", "2500", "-s1", "1", "-s2", "1", "-t", "4", "-c"],
    "5000, 1, 1, 0, 4, 0, 1": ["./main", "5000", "-s1", "1", "-s2", "1", "-t", "4", "-c"],
    # different sizes with sparcity 0 and threading and simd
    "500, 0, 0, 0, 4, 1, 0": ["./main", "500", "-t", "4", "-s"],
    "1000, 0, 0, 0, 4, 1, 0": ["./main", "1000", "-t", "4", "-s"],
    "2500, 0, 0, 0, 4, 1, 0": ["./main", "2500", "-t", "4", "-s"],
    "5000, 0, 0, 0, 4, 1, 0": ["./main", "5000", "-t", "4", "-s"],
    # different sizes with sparcity 1 and threading and simd
    "500, 1, 1, 0, 4, 1, 0": ["./main", "500", "-s1", "1", "-s2", "1", "-t", "4", "-s"],
    "1000, 1, 1, 0, 4, 1, 0": ["./main", "1000", "-s1", "1", "-s2", "1", "-t", "4", "-s"],
    "2500, 1, 1, 0, 4, 1, 0": ["./main", "2500", "-s1", "1", "-s2", "1", "-t", "4", "-s"],
    "5000, 1, 1, 0, 4, 1, 0": ["./main", "5000", "-s1", "1", "-s2", "1", "-t", "4", "-s"],
    # different sizes with sparcity 0 and cache and simd
    "500, 0, 0, 0, 1, 1, 1": ["./main", "500", "-sc"],
    "1000, 0, 0, 0, 1, 1, 1": ["./main", "1000", "-sc"],
    "2500, 0, 0, 0, 1, 1, 1": ["./main", "2500", "-sc"],
    "5000, 0, 0, 0, 1, 1, 1": ["./main", "5000", "-sc"],
    # different sizes with sparcity 1 and cache and simd
    "500, 1, 1, 0, 1, 1, 1": ["./main", "500", "-s1", "1", "-s2", "1", "-sc"],
    "1000, 1, 1, 0, 1, 1, 1": ["./main", "1000", "-s1", "1", "-s2", "1", "-sc"],
    "2500, 1, 1, 0, 1, 1, 1": ["./main", "2500", "-s1", "1", "-s2", "1", "-sc"],
    "5000, 1, 1, 0, 1, 1, 1": ["./main", "5000", "-s1", "1", "-s2", "1", "-sc"],
    # different sizes with sparcity 0 and threading and cache and simd
    "500, 0, 0, 0, 4, 1, 1": ["./main", "500", "-t", "4", "-sc"],
    "1000, 0, 0, 0, 4, 1, 1": ["./main", "1000", "-t", "4", "-sc"],
    "2500, 0, 0, 0, 4, 1, 1": ["./main", "2500", "-t", "4", "-sc"],
    "5000, 0, 0, 0, 4, 1, 1": ["./main", "5000", "-t", "4", "-sc"],
    # different sizes with sparcity 1 and threading and cache and simd
    "500, 1, 1, 0, 4, 1, 1": ["./main", "500", "-s1", "1", "-s2", "1", "-t", "4", "-sc"],
    "1000, 1, 1, 0, 4, 1, 1": ["./main", "1000", "-s1", "1", "-s2", "1", "-t", "4", "-sc"],
    "2500, 1, 1, 0, 4, 1, 1": ["./main", "2500", "-s1", "1", "-s2", "1", "-t", "4", "-sc"],
    "5000, 1, 1, 0, 4, 1, 1": ["./main", "5000", "-s1", "1", "-s2", "1", "-t", "4", "-sc"],
    # different sizes with sparcity 0.1 and sparse compression
    "500, 0.1, 0.1, 1, 1, 0, 0": ["./main", "500", "--sparse", "-s1", "0.1", "-s2", "0.1"],
    "1000, 0.1, 0.1, 1, 1, 0, 0": ["./main", "1000", "--sparse", "-s1", "0.1", "-s2", "0.1"],
    "2500, 0.1, 0.1, 1, 1, 0, 0": ["./main", "2500", "--sparse", "-s1", "0.1", "-s2", "0.1"],
    "5000, 0.1, 0.1, 1, 1, 0, 0": ["./main", "5000", "--sparse", "-s1", "0.1", "-s2", "0.1"],
    # different sizes with sparcity 1 and sparse compression
    "500, 1, 1, 1, 1, 0, 0": ["./main", "500", "--sparse", "-s1", "1", "-s2", "1"],
    "1000, 1, 1, 1, 1, 0, 0": ["./main", "1000", "--sparse", "-s1", "1", "-s2", "1"],
    "2500, 1, 1, 1, 1, 0, 0": ["./main", "2500", "--sparse", "-s1", "1", "-s2", "1"],
    "5000, 1, 1, 1, 1, 0, 0": ["./main", "5000", "--sparse", "-s1", "1", "-s2", "1"],
    # different sizes with sparcity 5 and sparse compression
    "500, 5, 5, 1, 1, 0, 0": ["./main", "500", "--sparse", "-s1", "5", "-s2", "5"],
    "1000, 5, 5, 1, 1, 0, 0": ["./main", "1000", "--sparse", "-s1", "5", "-s2", "5"],
    "2500, 5, 5, 1, 1, 0, 0": ["./main", "2500", "--sparse", "-s1", "5", "-s2", "5"],
    "5000, 5, 5, 1, 1, 0, 0": ["./main", "5000", "--sparse", "-s1", "5", "-s2", "5"],
    # different sizes with sparcity 0.1 for one and sparse compression
    "500, 0.1, 0, 1, 1, 0, 0": ["./main", "500", "--sparse", "-s1", "0.1"],
    "1000, 0.1, 0, 1, 1, 0, 0": ["./main", "1000", "--sparse", "-s1", "0.1"],
    "2500, 0.1, 0, 1, 1, 0, 0": ["./main", "2500", "--sparse", "-s1", "0.1"],
    "5000, 0.1, 0, 1, 1, 0, 0": ["./main", "5000", "--sparse", "-s1", "0.1"],
    # different sizes with sparcity 1 and sparse compression
    "500, 1, 0, 1, 1, 0, 0": ["./main", "500", "--sparse", "-s1", "1"],
    "1000, 1, 0, 1, 1, 0, 0": ["./main", "1000", "--sparse", "-s1", "1"],
    "2500, 1, 0, 1, 1, 0, 0": ["./main", "2500", "--sparse", "-s1", "1"],
    "5000, 1, 0, 1, 1, 0, 0": ["./main", "5000", "--sparse", "-s1", "1"],
    # different sizes with sparcity 5 and sparse compression
    "500, 5, 0, 1, 1, 0, 0": ["./main", "500", "--sparse", "-s1", "5"],
    "1000, 5, 0, 1, 1, 0, 0": ["./main", "1000", "--sparse", "-s1", "5"],
    "2500, 5, 0, 1, 1, 0, 0": ["./main", "2500", "--sparse", "-s1", "5"],
    "5000, 5, 0, 1, 1, 0, 0": ["./main", "5000", "--sparse", "-s1", "5"],
}
# fmt: on


def parse_perf(result):
    for line in result.split("\n"):
        if "task-clock" not in line:
            continue
        line = line.split()
        return float(line[0].replace(",", ""))


def run(command):
    return subprocess.run(command, encoding="utf-8", capture_output=True).stderr


try:
    os.remove(LOG_FILE)
except FileNotFoundError:
    pass


# compile
run(["g++", "main.cpp", "matrixmult.cpp", "-o", "main", "-Wall", "-msse3", "-03"])

with open(LOG_FILE, "w", encoding="utf-8") as fp:
    fp.write(
        "Matrix Size, Sparcity 1, Sparcity 2, Sparse Compression, Threads, SIMD, Cache, Execution Time\n"
    )
    for description, command in COMMANDS.items():
        print(command)
        result = run(["perf", "stat", "--"] + command)
        time = parse_perf(result)
        fp.write(f"{description}, {time}\n")
