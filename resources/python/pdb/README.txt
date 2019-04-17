\\
\\ Copyright (c) 2017 NTT corp. - All Rights Reserved
\\
\\  This file is part of Opcount project which is released under Software
\\  License Agreement for Evaluation. See file LICENSE.pdf for full license
\\  details.
\\

This folder stores performance database files.

Usage:
opcount <libname_curvename.pdb> <libname_common.pdb> -t <scheme.gp> < <scheme.csdl>



BD17 PDB
This database is for BN/BLS12/KSS16/KSS18 curves at 128bit security level and KSS18/BLS24 curves at 192bit security level. The performance is measured by complexity estimation in the number of 32-bit multiplications. The parameters are set to achieve targeted security level taking the Kim-Barbulescu attack into consideration.


KKKIYT17 PDB
Benchmark by Kiyomura et. al. for 256-bit security level. 
This database is obsolete. An updated benchmark is Kiyo17.


Kiyo17 PDB
This benchmark is for curves in 256 bit security level. It includes BLS and KSS curves with embedding degrees of 24/32/36/42/48. The timing is by clock counts based on an actual measurement on Core i7-6700 @ 3.4GHz.


RV15 PDB
This benchmark is for BN curves with group order 254/446/638 that correspond to 128/192/256bit security levels when the Kim-Barbulescu attack is not considered. The timing is by clock counts based on actual measurment on Arndale Board (ARM v7), Cortex-A15 at 1.7 GHz. Among many implementations compared in the article, the one with the best performance is included in the PDB.

KH17 PDB
Based on the same library as BD17. Benchmark on a new platform; Xeon CPU E5-2699 v4. The real timing data is provided for arithmetics in G1, which was missing in Kiyo17.



