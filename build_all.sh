#!/usr/bin/env sh


for example in `ls -d */`; do
	# avoid target directory
	echo "entering: $example"
	cd $example
	yotta build
	cd ..
done 

