#!/bin/bash

SRC=../src
EXEC=${SRC}/intersectionLignes
HISTDIR=../hist
IMAGE=../img

if [ ! -e ../hist/*.hist ];
then
	echo "No histograms dude"
fi

if [ ! -e ${EXEC} ];
then
	make -C ${SRC};
fi

for image in ../img/*.jpg;
do
  ${EXEC} ${HISTDIR}/*.hist ${image};
done
