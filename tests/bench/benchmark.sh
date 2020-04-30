#!/bin/bash

readonly FORTHS=( "gforth" "pforth" "simforth" ) # "simtadyn" )
OUTPUT='benchmark_results.csv'
PFORTH_DIR="/home/qq/pforth/build/unix"
SIMFORTH_DIR="/home/qq/MyGitHub/SimForth/build"
SIMTADYN_DIR="/home/qq/MyGitHub/SimTaDyn/src/forth/standalone/build"

echo -n "file" > $OUTPUT
for FORTH in "${FORTHS[@]}"
do
    echo -n ",$FORTH" >> $OUTPUT
done

for FILE in `ls *.fth`
do
    echo "Benchmarking $FILE ..."

    echo -ne "\n$FILE" >> $OUTPUT
    for FORTH in "${FORTHS[@]}"
    do
	COMMAND=
	case ${FORTH} in
	    gforth) COMMAND="${FORTH} $(pwd)/${FILE} -e bye";;
	    pforth) COMMAND="${PFORTH_DIR}/${FORTH}_standalone -q $(pwd)/${FILE}";;
	    simforth) COMMAND="${SIMFORTH_DIR}/SimForth -f $(pwd)/${FILE}";;
	    simtadyn) COMMAND="${SIMTADYN_DIR}/SimForth -f ${SIMTADYN_DIR}/../../core/system.fs -f $(pwd)/${FILE}";;
	    *) ;;
	esac

	echo "  ${FORTH}"
	#echo "  Command: ${COMMAND}"
	/usr/bin/time -f ",%E" -o foo ${COMMAND} > /dev/null 2>&1
	cat foo | tr -d '\n' >> $OUTPUT
	rm -f foo
    done
done
