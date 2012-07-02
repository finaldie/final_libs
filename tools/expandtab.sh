#!/bin/sh

if [ $# != 1 ]; then
    echo "must input process dir";
else
    process_files=".process_files.txt";
    find $1 -name "*.h" -o -name "*.c" > $process_files
    list=`cat $process_files`
    for one in $list
    do
        sed 's/	/    /g' $one > $one.bak
        mv $one.bak $one
    done
    rm -f $process_files

    echo "process $1 all the files complete, please make a diff for checking that";
fi
