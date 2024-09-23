#!/usr/bin/env python

import subprocess
import os


def run_mlc(buffer_size, fp):
    result = subprocess.run(
        ["mlc", "--idle_latency", f"-b{buffer_size}", "-r"],
        encoding="utf-8",
        capture_output=True,
    )
    # find the time
    out = result.stdout
    out = out[out.find("Each iteration") :]
    out = out[out.find("(") + 1 : out.find(")")]
    fp.write(f"{buffer_size}, {out}\n")


try:
    os.remove("log.txt")
except FileNotFoundError:
    pass


with open("log.txt", "w", encoding="utf-8") as fp:
    # the buffer_size is given in KiB (so 200 KiB to 200,000 KiB)
    for buffer_size in range(200, 20_000, 200):
        run_mlc(buffer_size, fp)
    for buffer_size in range(20_000, 200_000, 20_000):
        run_mlc(buffer_size, fp)
