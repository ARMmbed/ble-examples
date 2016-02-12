#!/usr/bin/env sh


for example in `ls -d */`; do
	# avoid target directory
	if [ $example = "targets/" ]
	then 
		continue
	fi

	echo "entering: $example"
	cd $example
	yotta build
	cd ..
done 

