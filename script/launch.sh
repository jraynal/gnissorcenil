#!/bin/bash

SRC=../src
EXEC=${SRC}/intersectionLignes
HISTDIR=../hist
IMGDIR=../img

if [ ! -e ../hist/*.hist ];
then
	echo "No histograms dude"
fi

if [ ! -e ${EXEC} ];
then
	make -C ${SRC};
fi

HISTOS=""
for histo in ${HISTDIR}/*.hist;
do
	HISTOS=${HISTOS}"${histo} ";
done
echo ${HISTOS};

for image in ${IMGDIR}/*.jpg;
do
  ${EXEC} ${image};
done
