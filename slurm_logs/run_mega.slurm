#!/bin/bash
#SBATCH --job-name=birthday_test
#SBATCH --account=cits
#SBATCH --partition=cits3402
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=8
#SBATCH --mem=4G
#SBATCH --time=01:30:00
#SBATCH --output=birthday_%j.out
#SBATCH --error=birthday_%j.err
#SBATCH --mail-type=END,FAIL
#SBATCH --mail-user=25053306@student.uwa.edu.au

echo "Job ID: $SLURM_JOB_ID"
echo "Running on: $(hostname)"
echo "CPUs: $SLURM_CPUS_PER_TASK"

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
export OMP_DYNAMIC=false

./collision_finder \
    /group/cits/cits123/cits3402-assignment1-data/2_mega_a.pdf \
    /group/cits/cits123/cits3402-assignment1-data/2_mega_b.pdf \
    $SLURM_CPUS_PER_TASK
