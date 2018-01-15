#! /bin/sh
# .sh file for BD17 timing data
#
# Copyright (c) 2017 NTT corp. - All Rights Reserved
#
#  This file is part of Opcount project which is released under Software
#  License Agreement for Evaluation. See file LICENSE.pdf for full license
#  details.
#


if [ $# -le 2 ]; then
    echo "Usage: $0 <csdl_file> <gp_file> <curve1> [curve2] ..." 1>&2
    echo "possible curves: BN_128, BLS12_128, KSS16_128, KSS18_128, BLS24_192, KSS18_192"
    exit 1
fi

pdbdir="../../pdb"
opcount="../../opcount/opcount"
csdlfile=$1
gpfile=$2


if [ ! -x "$opcount" ]; then
    echo "not executable: $opcount" 1>&2
    exit 1;
fi

if [ ! -e "$pdbdir/BD17_common.pdb" ]; then
    echo "file not found: $pdbdir/BD17_common.pdb" 1>&2
    exit 1;
fi


shift
shift
args=$*

for curve in $args
do
    if [ ! -e "$pdbdir/BD17_$curve.pdb" ]; then
	echo "file not found: $pdbdir/BD17_$curve.pdb" 1>&2
	exit 1;
    fi

    echo
    echo "Evaluating $csdlflie on $curve."
    $opcount $pdbdir/BD17_$curve.pdb $pdbdir/BD17_common.pdb -t $gpfile < $csdlfile | gp -q
done
