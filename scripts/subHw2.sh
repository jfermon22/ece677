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

EXEC=~/ece677/build_dir/$EXEC_DIR/$EXEC_NAME
OUTPUT=$EXEC_NAME\_out
OUTPUT_CHECK=$EXEC_NAME\_serial.txt

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
   
   sed -i -e '1d;$d' tmp$OUTPUT_CHECK
   sed -i -e '1d;$d' tmp$OUTPUT

   if [[ `diff tmp$OUTPUT_CHECK tmp$OUTPUT` == "" ]]; then
       echo "$NODES::Success"
       echo "$NODES::Success" > $EXEC_NAME\_status_$NODES.txt
   else
       echo "$NODES::Fail"
       echo "$NODES::Fail" > $EXEC_NAME\_status_$NODES.txt
   fi
   
   mv $OUTPUT $OUTPUT_DIR/$OUTPUT\_$NODES.txt
   mv $EXEC_NAME\_status_$NODES.txt $OUTPUT_DIR
   
   rm tmp$OUTPUT tmp$OUTPUT_CHECK
   
   for FILE in  $EXEC_NAME.e* $EXEC_NAME.o* $OUTPUT; do
      if [ -e $FILE ]; then
         rm $FILE
      fi
   done

done

mv  $OUTPUT_CHECK $OUTPUT_DIR/

done
