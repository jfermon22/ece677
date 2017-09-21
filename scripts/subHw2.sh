###========================================
#!/bin/bash
# Your job will use 3 nodes, 66 cores, and 396gb of memory total.
#PBS -q windfall
###PBS -l select=3:ncpus=22:mem=1gb:pcmem=6gb
#PBS -l select=4:ncpus=16:mem=10gb
### Specify a name for the job
#PBS -N hw2
### Specify the group name
#PBS -W group_list=ece677fall17
### Used if job requires partial node only
#PBS -l place=free:shared
### CPUtime required in hhh:mm:ss.
### Leading 0's can be omitted e.g 48:0:0 sets 48 hours
#PBS -l cput=0:1:0
### Walltime is created by cputime divided by total cores.
### This field can be overwritten by a longer time
#PBS -l walltime=00:05:00

###Request email when job begins and ends
###PBS -m bea
###specify email address to use for notification
#PBS -M fermon@email.arizona.edu
#---------------------------------------------------------------------


###Load required modules if needed
### use 'module avail' for list of available modules
module load openmpi

EXEC_DIR="hw2"
DATE=`date +%Y%m%d_%H%M%S`
OUTPUT_DIR=~/ece677/outputs/$EXEC_DIR\_$DATE
mkdir -p $OUTPUT_DIR

for EXEC_NAME in "matrix_multiply" "mean_filter"
do

BASELINE_EXEC_NAME=$EXEC_NAME\_hw2
BASELINE_EXEC=~/ece677/build_dir/hw2/$BASELINE_EXEC_NAME
EXEC_NAME=$EXEC_NAME\_$EXEC_DIR
EXEC=~/ece677/build_dir/$EXEC_DIR/$EXEC_NAME
OUTPUT=$EXEC_NAME\_out
BASELINE_OUT=$BASELINE_EXEC_NAME\_serial.txt

$BASELINE_EXEC --serial > $BASELINE_OUT
for NODES in 02 04 08 16 32 64
do
   
   for FILE in  $EXEC_NAME.e* $EXEC_NAME.o* $OUTPUT; do
      if [ -e $FILE ]; then
         rm $FILE
      fi
   done

   mpirun -n $NODES $EXEC > $OUTPUT
   
   cp $BASELINE_OUT tmp$BASELINE_OUT
   cp $OUTPUT tmp$OUTPUT
   
   sed -i -e '1d;$d' tmp$BASELINE_OUT
   sed -i -e '1d;$d' tmp$OUTPUT

   if [[ `diff tmp$BASELINE_OUT tmp$OUTPUT` == "" ]]; then
       echo "$NODES::Success"
       echo "$NODES::Success" > $EXEC_NAME\_status_$NODES.txt
   else
       echo "$NODES::Fail"
       echo "$NODES::Fail" > $EXEC_NAME\_status_$NODES.txt
   fi
   
   mv $OUTPUT $OUTPUT_DIR/$OUTPUT\_$NODES.txt
   mv $EXEC_NAME\_status_$NODES.txt $OUTPUT_DIR
   
   rm tmp$OUTPUT tmp$BASELINE_OUT
   
   for FILE in  $EXEC_NAME.e* $EXEC_NAME.o* $OUTPUT; do
      if [ -e $FILE ]; then
         rm $FILE
      fi
   done

done

mv  $BASELINE_OUT $OUTPUT_DIR/

done
