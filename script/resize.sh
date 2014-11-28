#!/bin/bash

HIMGDIR=../img/HighReso
MIMGDIR=../img/MedReso
LIMGDIR=../img/LowReso

mkdir -p ${MIMGDIR};
mkdir -p ${LIMGDIR};

for image in ${HIMGDIR}/*.jpg;
do
  BASE=$(basename ${image});
  convert -resize 50% ${image} ${MIMGDIR}/${BASE};
  convert -resize 25% ${image} ${LIMGDIR}/${BASE};
done
