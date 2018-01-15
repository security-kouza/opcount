#! /bin/bash
##
## Copyright (c) 2017 NTT corp. - All Rights Reserved
##
##  This file is part of Opcount project which is released under Software
##  License Agreement for Evaluation. See file LICENSE.pdf for full license
##  details.
##

echo "Testing accuracy of estimation for TightSPS over KH17 pDB."
echo "Error_rate = (benchmark - estimation) / (benchmark)"
echo "(Negative value means the real data is faster.)"

pdbdir=../../pdb
csdldir=../../csdl
opc=../../opcount/opcount
pdb=()
pdb=("BLS24" "KSS32" "KSS36" "BLS42" "BLS48")


for curve in ${pdb[@]}; do
    echo
    echo "Curve = $curve"
    $opc $pdbdir/KH17_"$curve"_256.pdb $pdbdir/KH17_common.pdb tSPS_KH17_"$curve".pdb -t tightSPSerrtest.gp < $csdldir/tightSPS17.csdl | gp -q
done
