#!/bin/sh

#
# This is a simple script to automate the running of a bunch of instances of Turnstile.
# J.D. Henderson
#

rm -rf core.*

MAX=50
a=1

while [ "$a" -le $MAX ]
do
  	echo "Running $a"
	./turnstile auto "Private_Key_1$a" "Buddy$a" "600$a" &
	sleep 30
  	let "a+=1"
done


