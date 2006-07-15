#!/bin/sh

rm -rf core.*
cp -Rpf ../src/turnstile .
./turnstile

