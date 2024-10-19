#!/usr/bin/env python

import os
import subprocess
from pathlib import Path
import json

SCRIPT_DIR = Path(__file__).parent
OUTPUT_DIR = SCRIPT_DIR / "output"

IO_DEPTHS = [1, 4, 8, 32, 256, 1024]
BLOCK_SIZES = [2**i for i in range(2, 8)]  # 4..128
READ_PERCENTAGES = [0, 50, 70, 100]


def run(iodepth, block_size, read_percent):
    out_file = OUTPUT_DIR / f"output_qlen{iodepth}_bs{block_size}_rw{read_percent}.json"
    job_file = SCRIPT_DIR / "fio.ini"
    cmd = [
        "fio",
        "--output-format=json",
        f"--output={out_file}",
        f"--iodepth={iodepth}",
        f"--blocksize={block_size}k",
        f"--rwmixread={read_percent}",
        f"{job_file}",
    ]
    print(cmd)
    subprocess.run(cmd, encoding="utf-8")


def parse(file_name, result):
    segments = file_name.split(".")[0].split("_")[1:]
    qlen = int(segments[0][4:])
    bs = int(segments[1][2:])
    rw = int(segments[2][2:])

    stats = [
        result["jobs"][0]["read"]["bw"],
        result["jobs"][0]["read"]["iops"],
        result["jobs"][0]["read"]["lat_ns"]["mean"],
        result["jobs"][0]["write"]["bw"],
        result["jobs"][0]["write"]["iops"],
        result["jobs"][0]["write"]["lat_ns"]["mean"],
    ]
    stats = [str(s) for s in stats]

    return f"{bs}, {rw}:{100-rw}, {qlen}, " + ", ".join(stats)


try:
    os.mkdir(OUTPUT_DIR)
except FileExistsError:
    pass


# run all the necessary tests
combined = [(d, b, r) for d in IO_DEPTHS for b in BLOCK_SIZES for r in READ_PERCENTAGES]
for depth, bs, read in combined:
    run(depth, bs, read)

# parse the results of all the tests
print(
    "Data Access Size, RW Ratio, IO queue depth,"
    + " Read Bandwidth (KB/s), Read IOPS, Read Latency (ns),"
    + " Write Bandwidth (KB/s), Write IOPS, Write Latency (ns)"
)
for file in OUTPUT_DIR.glob("*.json"):
    with file.open("r", encoding="utf-8") as fp:
        data = json.load(fp)
    print(parse(file.name, data))
