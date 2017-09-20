###========================================
#!/bin/bash
# Your job will use 3 nodes, 66 cores, and 396gb of memory total.
#PBS -q windfall
#PBS -l select=3:ncpus=22:mem=1gb:pcmem=6gb
### Specify a name for the job
#PBS -N matrix_multiply
### Specify the group name
#PBS -W group_list=ece677fall17
### Used if job requires partial node only
#PBS -l place=pack:shared
### CPUtime required in hhh:mm:ss.
### Leading 0's can be omitted e.g 48:0:0 sets 48 hours
#PBS -l cput=000:00:10
### Walltime is created by cputime divided by total cores.
### This field can be overwritten by a longer time
#PBS -l walltime=00:00:00

###Request email when job begins and ends
#PBS -m bea
###specify email address to use for notification
#PBS -M fermon@email.arizona.edu
#---------------------------------------------------------------------



###Load required modules if needed
### use 'module avail' for list of available modules
module load openmpi

EXEC_DIR="hw2"
EXEC_NAME="mean_filter"
EXEC=~/ece677/build_dir/$EXEC_DIR/$EXEC_NAME
OUTPUT=$EXEC_NAME\_out
OUTPUT_CHECK=$EXEC_NAME\_serial.txt
OUTPUT_DIR=$EXEC_NAME_`date +%Y%m%d_%H%S`

mkdir $OUTPUT_DIR

$EXEC --serial > $OUTPUT_CHECK

for NODES in 02 04 08 16 32 64
do

   for FILE in  $EXEC_NAME.e* $EXEC_NAME.o* $OUTPUT; do
      if [ -e $FILE ]; then
         rm $FILE
      fi
   done

   mpirun -n $NODES $EXEC > $OUTPUT
   
   cp $OUTPUT_CHECK tmp$OUTPUT_CHECK
   cp $OUTPUT tmp$OUTPUT
   
   sed -i -e "1d" tmp$OUTPUT_CHECK
   sed -i -e '1d;$d' tmp$OUTPUT

   if [[ `diff tmp$OUTPUT_CHECK tmp$OUTPUT` == "" ]]; then
       echo "$NODES::Success"
       echo "$NODES::Success" > status_$NODES.txt
   else
       echo "$NODES::Fail"
       echo "$NODES::Fail" > status_$NODES.txt
   fi
   
   mv $OUTPUT $OUTPUT_DIR/$OUTPUT\_$NODES.txt
   mv status_$NODES.txt $OUTPUT_DIR
   
   rm tmp$OUTPUT tmp$OUTPUT_CHECK

done
