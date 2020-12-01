nIWANT=`grep IWANT u.log | wc -l`;
nRECVD=`grep RECVD q.log | wc -l`;
nENTER=`grep ENTER q.log | wc -l`;
nIAMIN=`grep IAMIN u.log | wc -l`;
nTIMUP=`grep TIMUP q.log | wc -l`;
nGAVUP=`grep GAVUP q.log | wc -l`;
n2LATE=`grep 2LATE q.log | wc -l`;
nCLOSD=`grep CLOSD u.log | wc -l`;

echo IWANT = $nIWANT;
echo RECVD = $nRECVD;
echo ENTER = $nENTER;
echo IAMIN = $nIAMIN;
echo TIMUP = $nTIMUP;
echo GAVUP = $nGAVUP;
echo 2LATE = $n2LATE;
echo CLOSD = $nCLOSD;