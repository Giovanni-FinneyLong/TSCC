
#!/bin/bash

# Which queue to be used
#PBS -q hotel
# Name of the job
#PBS -N 12500_1
# How many machines and # of cores per machine
#PBS -l nodes=1:ppn=2
# The name of regular output file.
#PBS -o 12500_1.out
# The name of error output file.
#PBS -e 12500_1.err
# set the maximum long time
#PBS -l walltime=02:00:00
# the account to charge my usage. Use your login name
#PBS -A ucsb-train21
#PBS -V
# the email to notify  the job status
#PBS -M giorulesall.at@gmail.com
# send an email at the beginning and the end of your job and also if there is an error
#PBS -m abe

# Then you run "qsub example.sh" to execute this file in a queue
# You may do "qsub -I", and then "sh example.sh" as an interactive job

#cd ~/output
mpirun -v -machinefile $PBS_NODEFILE -np 32 ./oddEvenSort


