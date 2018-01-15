## .sh file for batch test.
##
## Copyright (c) 2017 NTT corp. - All Rights Reserved
##
##  This file is part of Opcount project which is released under Software
##  License Agreement for Evaluation. See file LICENSE.pdf for full license
##  details.
##

#! /bin/sh


csdldir=../../csdl
gpdir=.

echo "Estimating performance of tightSPS17 based on DB17"
echo
echo "Naiive:"
./BD17.sh $csdldir/tightSPS17.csdl $gpdir/tightSPS17.gp BN_128 BLS12_128 KSS16_128 KSS18_128

echo
echo "Batch only in G0"
./BD17.sh $csdldir/tightSPS17_g0batch.csdl $gpdir/tightSPS17.gp BN_128 BLS12_128 KSS16_128 KSS18_128

echo
echo "Full Batch"
./BD17.sh $csdldir/tightSPS17_batch.csdl $gpdir/tightSPS17.gp BN_128 BLS12_128 KSS16_128 KSS18_128

