#!/bin/bash
# Copyright (c) 2015 Matthias Noack <ma.noack.pr@gmail.com>
#
# See accompanying file LICENSE and README for further information.

# replace the fourth column of each data file with a sequence as particle id 
# filter inactive particles (last column is 1.0)
# append everything into a single file

DATA_PATH=$1

cd $DATA_PATH
RESULT_FILE="merged.dat"
echo -n "" > $RESULT_FILE # start fresh

FILES=$(ls p{000000..010000..100}.dat)
#FILES=p000000.dat
#echo $FILES
echo "Merging $(echo "$FILES" | wc -l) files..."

for file in $FILES
do
#	echo $file
	LINES=$(wc -l $file | cut -d ' ' -f1) # number of lines reported by wc
	cut -d ' ' -f1-3 $file > ${file}.tmp1
	seq 1 $LINES | paste -d ' ' ${file}.tmp1 - > ${file}.tmp2 # '-' means rad from stdin
#	echo "Lines: $LINES"
	cut -d ' ' -f1-4 --complement ${file} | paste -d ' ' ${file}.tmp2 - > ${file}.tmp1 # paste the remaining columns
	# filter inactive
	grep -v "1.000000$" ${file}.tmp1 > ${file}.tmp2 # grep lines not (-v) ending ($) with 1.00000 
	cat ${file}.tmp2 >> ${RESULT_FILE}.tmp
	rm ${file}.tmp1 ${file}.tmp2
done
echo "... done."
RESULT_ENTRIES=$(wc -l ${RESULT_FILE}.tmp  | cut -d ' ' -f1)

sort -s -g -k 4 ${RESULT_FILE}.tmp > ${RESULT_FILE}.tmp2 # messes up chronological order without -s

echo "$RESULT_ENTRIES" > ${RESULT_FILE}
cat ${RESULT_FILE}.tmp2 >> ${RESULT_FILE}
rm -f ${RESULT_FILE}.tmp ${RESULT_FILE}.tmp2
echo "Result entries: $RESULT_ENTRIES"
echo "Filtered entries: $(grep "1.000000$" $FILES | wc -l)"

cd ..
