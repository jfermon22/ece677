#!/bin/bash

cd ../build_dir/hw2/

if ( ! make ); then
    echo "failed to make"
    exit 1
fi

./mean_filter --serial > tmpserial.txt

mpirun -n 10 mean_filter > tmpparallel.txt

sed -i -e "1d" tmpserial.txt
sed -i -e "1d" tmpparallel.txt

if [[ `diff tmpserial.txt tmpparallel.txt` == "" ]]; then
    echo "Success"
else
    echo "Fail"
fi
