#!/usr/bin/env python

import subprocess
import os


def run_mlc(fp):
    result = subprocess.run(
        ["mlc", "--loaded_latency", "-e", "-r"],
        encoding="utf-8",
        capture_output=True,
    )
    # parse the output
    lines = result.stdout.split("\n")
    index = 0
    for i in range(len(lines)):
        if lines[i].find("=") != -1:
            index = i + 1
            break
    for i in range(index, len(lines)):
        line = lines[i].split()
        if len(line) == 0:
            continue
        delay = line[0]
        latency = line[1]
        bandwidth = line[2]
        fp.write(f"{delay}, {latency}, {bandwidth}\n")


try:
    os.remove("log.txt")
except FileNotFoundError:
    pass


with open("log.txt", "w", encoding="utf-8") as fp:
    run_mlc(fp)
