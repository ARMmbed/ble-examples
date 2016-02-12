#!/usr/bin/env sh

export BLE_EXAMPLES_ROOT=$PWD

copy_repository() { 
	src=$1
	dest=$2
	if [ -d ${dest} ] 
	then
		rm -rf ${dest}
	fi
	git clone ${src} ${dest}
}


# create targets repository
if [ -d "targets" ]
then
	rm -rf targets
fi

mkdir targets
cd targets

# clone targets
copy_repository "https://github.com/ARMmbed/target-nrf52dk-gcc.git" "nrf52dk-gcc"
copy_repository "https://github.com/ARMmbed/nordic-nrf52832-gcc.git" "nordic-nrf52832-gcc"

# create a links directory to not mess with other yt symbolinc links
mkdir yotta_links
export YOTTA_PREFIX="$PWD/yotta_links"

# create links for target
cd nrf52dk-gcc
yotta link-target 
cd ../nordic-nrf52832-gcc
yotta link-target 

cd $BLE_EXAMPLES_ROOT


# link all examples with the right target
for example in `ls -d */`; do
	# avoid target directory
	if [ $example = "targets/" ] 
	then
		continue
	fi

	echo "entering: $example"
	cd $example
	yotta link-target nrf52dk-gcc
	yotta link-target nordic-nrf52832-gcc
	yotta target nrf52dk-gcc
	cd ..
done 

