###========================================
#!/bin/bash
###script to run an mpi job using 12-core or less (using only one 12 core node)
###set the job name
#PBS -N matrix_multiply
###specify the PI group for this job
###List of PI groups available to each user can be found with "va" command
#PBS -W group_list=ece677fall17
###Request email when job begins and ends
###PBS -m bea
###specify email address to use for notification
#PBS -M fermon@email.arizona.edu
###set the queue
#PBS -q windfall
#Set the number of nodes, cores, and memory that will be used for this job
#PBS -l select=1:ncpus=28:mem=1kb
###specify total cpu time required for this job, hhh:mm:ss
#PBS -l walltime=0:5:0
###total cputime = walltime * ncpus
#PBS -l cput=0:28:0
#PBS -l place=free:shared
###Load required modules if needed
### use 'module avail' for list of available modules
module load openmpi
#---------------------------------------------------------------------
#

EXEC_DIR="hw2"
EXEC="matrix_multiply"


cd ~/ece677/build_dir/$EXEC_DIR

date 
mpirun -n 16 $EXEC
date 
