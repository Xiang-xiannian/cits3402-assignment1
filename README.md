# CITS3402/CITS5507 Assignment 1 — Parallel Birthday Attack on toy_hash

Nan Xiang (25053306)

## Project structure

├── main.c              # Entry point: loads files, runs search, verifies, writes output
├── pdf_io.c / .h        # PDF file I/O and header field editing (nonce, student ID)
├── toy_hash.c / .h      # Reference hash function (provided, unmodified)
├── collision.c / .h     # Parallel birthday-attack search + partitioned hash table
├── Makefile
├── data/                # Provided input PDF pairs
├── slurm_logs/          # Slurm scripts and job output logs used from Kaya experiments
├── solved_*.pdf         # Final solved file pairs for all six provided pairs
├── check_toy_hash.py    # Provided Python reference implementation (for verification)
└── CITS3402_Assignment1_Report.pdf # Performance summary and analysis report
## Building

    make

Produces an executable named `collision_finder`. Requires a C compiler with OpenMP
support (`gcc -fopenmp`).

## Running locally

    ./collision_finder <file_a.pdf> <file_b.pdf> [num_threads]

`num_threads` is optional; defaults to the system's maximum available threads if omitted.

Example:

    ./collision_finder data/1_kilo_a.pdf data/1_kilo_b.pdf 8

On success, this writes `solved_1_kilo_a.pdf` and `solved_1_kilo_b.pdf` in the
current directory (output filenames are derived automatically from the input
filenames), and prints the search time, the discovered nonces, and a
verification check against a freshly recomputed hash.

## Running on Kaya (Slurm)

Each provided file pair was benchmarked across thread counts
{1, 2, 4, 8, 16, 32, 48, 64, 96} on a single Kaya node. All scripts and raw
output logs used for these experiments are kept in `slurm_logs/`.

Example: submitting a single run manually

    sbatch --cpus-per-task=64 --export=ALL,PAIR=1_kilo --job-name=solve_1_kilo run_solve_one.slurm


## Verifying a solved pair

    python check_toy_hash.py solved_1_kilo_a.pdf
    python check_toy_hash.py solved_1_kilo_b.pdf

Both should print the same 12-character hexadecimal hash.

## Performance summary

Full timing results, speedup analysis, and discussion of the parallelisation
and data structure design are provided in the accompanying report
(`CITS3402_Assignment1_Report.pdf`). Raw wall-clock times for every (file pair, thread count)
combination are recorded in `slurm_logs/`.