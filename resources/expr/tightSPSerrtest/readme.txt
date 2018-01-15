##
## Copyright (c) 2017 NTT corp. - All Rights Reserved
##
##  This file is part of Opcount project which is released under Software
##  License Agreement for Evaluation. See file LICENSE.pdf for full license
##  details.
##

HK17 PDB を更新して誤差測定実験をやり直し。
- tightSPS の署名生成がスカラー値 0 の演算を実行して余計な時間を掛けていた点を修正
- cash の効果を測るため KSS36 については on cash 時のベンチマークで作成した PDB を追加
