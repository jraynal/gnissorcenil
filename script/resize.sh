#!/bin/bash

HIMGDIR=../img/HighReso
HCIMGDIR=../img/HighReso/Clipped
MIMGDIR=../img/MedReso
MCIMGDIR=../img/MedReso/Clipped
LIMGDIR=../img/LowReso
LCIMGDIR=../img/LowReso/Clipped

mkdir -p ${MIMGDIR};
mkdir -p ${MCIMGDIR};
mkdir -p ${LIMGDIR};
mkdir -p ${LCIMGDIR};

for image in ${HIMGDIR}/*.jpg;
do
  BASE=$(basename ${image});
  convert -resize 50% ${image} ${MIMGDIR}/${BASE};
  convert -resize 25% ${image} ${LIMGDIR}/${BASE};
done

for image in ${HCIMGDIR}/*.jpg;
do
  BASE=$(basename ${image});
  convert -resize 50% ${image} ${MCIMGDIR}/${BASE};
  convert -resize 25% ${image} ${LCIMGDIR}/${BASE};
done
