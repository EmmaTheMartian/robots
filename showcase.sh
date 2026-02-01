#!/usr/bin/env sh
set -e

sh run.sh -n

showcase() {
	args="-p $1"
	line=$(head -1 $1)
	if [ "$(echo $line | cut -f2 -d";")" = "args" ]
	then
		args="$args $(echo $line | cut -f3 -d';')"
	fi
	base=$(basename $1)
	id="${base%.rbt}"
	args="$args -S -s $id"
	echo "showcase: $id: $args"
	./robots $args &
}

if [ $# -gt 0 ]
then
	for f in "$@"
	do
		showcase showcases/$f.rbt
	done
else
	for f in `find showcases/ -name "*.rbt"`
	do
		showcase $f
	done
fi
