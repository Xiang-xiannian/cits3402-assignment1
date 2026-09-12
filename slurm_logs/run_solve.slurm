#!/bin/bash
#SBATCH --job-name=solve_${PAIR}
#SBATCH --account=cits
#SBATCH --partition=cits3402
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=32
#SBATCH --mem=4G
#SBATCH --time=00:20:00
#SBATCH --output=solve_%x_%j.out
#SBATCH --mail-type=END,FAIL
#SBATCH --mail-user=25053306@student.uwa.edu.au

module load gcc/9.4.0

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
export OMP_DYNAMIC=false

DATA_DIR="/group/cits/cits123/cits3402-assignment1-data"

./collision_finder "$DATA_DIR/${PAIR}_a.pdf" "$DATA_DIR/${PAIR}_b.pdf" $SLURM_CPUS_PER_TASK
