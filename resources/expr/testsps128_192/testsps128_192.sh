#! /bin/sh
#
## .sh files for estimating performance over 128 bit security and 192 bit security.
##
## Copyright (c) 2017 NTT corp. - All Rights Reserved
##
##  This file is part of Opcount project which is released under Software
##  License Agreement for Evaluation. See file LICENSE.pdf for full license
##  details.
##

echo "Estimating performance of some SPSes over DB17"
echo "with batch_quality=128 for all."

gpdir=.
csdldir=../../csdl

echo
echo "TightSPS (naiive) at 128bit level"
csdlfile="tightSPS17.csdl"
gpfile="tightSPS17.gp"
./BD17.sh $csdldir/$csdlfile $gpdir/$gpfile BN_128 BLS12_128 KSS16_128 KSS18_128


echo
echo "TightSPS (batch in G0) at 128bit level"
csdlfile="tightSPS17_g0batch.csdl"
gpfile="tightSPS17.gp"
./BD17.sh $csdldir/$csdlfile $gpdir/$gpfile BN_128 BLS12_128 KSS16_128 KSS18_128


echo
echo "TightSPS (full batch) at 128bit level"
csdlfile="tightSPS17_batch.csdl"
gpfile="tightSPS17.gp"
./BD17.sh $csdldir/$csdlfile $gpdir/$gpfile BN_128 BLS12_128 KSS16_128 KSS18_128


echo
echo "KPW15 (naiive) at 192bit level"
csdlfile="KPW15.csdl"
gpfile="KPW15.gp"
./BD17.sh $csdldir/$csdlfile $gpdir/$gpfile BLS24_192 KSS18_192


echo
echo "KPW15 (full batch) at 192bit level"
csdlfile="KPW15_batch.csdl"
gpfile="KPW15.gp"
./BD17.sh $csdldir/$csdlfile $gpdir/$gpfile BLS24_192 KSS18_192


echo
echo "JR17 (naiive) at 192bit level"
csdlfile="QlogQ.csdl"
gpfile="QlogQ.gp"
./BD17.sh $csdldir/$csdlfile $gpdir/$gpfile BLS24_192 KSS18_192

echo
echo "JR17 (full batch) at 192bit level"
csdlfile="QlogQ_batch_g0batch.csdl"
gpfile="QlogQ.gp"
./BD17.sh $csdldir/$csdlfile $gpdir/$gpfile BLS24_192 KSS18_192

