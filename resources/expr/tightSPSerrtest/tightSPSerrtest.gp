\\
\\ Copyright (c) 2017 NTT corp. - All Rights Reserved
\\
\\  This file is part of Opcount project which is released under Software
\\  License Agreement for Evaluation. See file LICENSE.pdf for full license
\\  details.
\\

erk = 100*(tSPSGen_implement - call_tSPSGen)/tSPSGen_implement;
ers = 100*(tSPSSign_implement - call_tSPSSign)/tSPSSign_implement;
erv = 100*(tSPSVerify2_implement - eval(call_tSPSVerify))/tSPSVerify2_implement;
print("error in KeyGen = ", eval(erk), "%", "(real=", eval(tSPSGen_implement), ", est=", eval(call_tSPSGen), ")");
print("error in Sign = ", eval(ers), "%", "(real=", eval(tSPSSign_implement), ", est=", eval(call_tSPSSign), ")");
print("error in Verify = ", eval(erv), "%", "(real=", eval(tSPSVerify2_implement), ", est=", eval(call_tSPSVerify), ")");

