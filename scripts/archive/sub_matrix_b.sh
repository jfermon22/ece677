###========================================
#!/bin/bash
#BSUB -n 1 
#BSUB -o lsf.out
#BSUB -e lsf.err
#BSUB -q "windfall"
#BSUB -J matrix_mul 
#---------------------------------------------------------------------
#BSUB -R gpu
#BSUB -R "span[ptile=2]"
module load openmpi
#---------------------------------------------------------------------
#

EXEC_DIR="hw2"
EXEC_NAME="matrix_multiply"
EXEC=~/ece677/build_dir/$EXEC_DIR/$EXEC_NAME
OUTPUT=$EXEC_NAME\_out
OUTPUT_CHECK=$EXEC_NAME\_serial.txt
OUTPUT_DIR=$EXEC_NAME_`date +%Y%m%d_%H%S`

mkdir $OUTPUT_DIR

$EXEC --serial > $OUTPUT_CHECK

for NODES in 02 04 08 16 32 64
do

   if [ -e $OUTPUT ]; then
       rm $OUTPUT
   fi 

   mpirun -np $NODES $EXEC > $OUTPUT
   
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
