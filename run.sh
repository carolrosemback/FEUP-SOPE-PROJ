
make;

echo running....;
./Q1 -t 30 -l 9 -n 10 fif > q.log 2> q.err & ./U1 -t 35 fif > u.log 2> u.err;
