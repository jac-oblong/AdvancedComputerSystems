#!/usr/bin/env python

import os
import subprocess
from pathlib import Path

SCRIPT_DIR = Path(__file__).parent
OUTPUT_DIR = SCRIPT_DIR / "output"

IO_DEPTHS = [2**i for i in range(11)]  # 1..1024
BLOCK_SIZES = [2**i for i in range(8)]  # 1..128
READ_PERCENTAGES = [0, 50, 70, 100]


def run(iodepth, block_size, read_percent):
    out_file = OUTPUT_DIR / f"output_qlen{iodepth}_bs{block_size}_rw{read_percent}.json"
    job_file = SCRIPT_DIR / "jobs.fio"
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


try:
    os.mkdir(OUTPUT_DIR)
except FileExistsError:
    pass


combined = [(d, b, r) for d in IO_DEPTHS for b in BLOCK_SIZES for r in READ_PERCENTAGES]
for depth, bs, read in combined:
    run(depth, bs, read)
