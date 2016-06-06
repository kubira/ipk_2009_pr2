#!/bin/ksh

make

./server -p 1243 &

./client -h eva.fit.vutbr.cz -p 1243 -NSLF
./client -h eva.fit.vutbr.cz -p 1243 -L -n Radim -s Kubis
./client -h eva.fit.vutbr.cz -p 1243 -NSL -f FIT
./client -p 1243 -h eva.fit.vutbr.cz -NS -l xkubis03
./client -F -l xkubis03 -h eva.fit.vutbr.cz -p 1243
./client -L -f FEKT -p 1243 -h eva.fit.vutbr.cz
./client -h eva.fit.vutbr.cz -p 1243 -n Radim
./client -h eva.fit.vutbr.cz -p 1243 -s Kubis
./client -h eva.fit.vutbr.cz -N -s Kubis -p 1243
./client -h eva.fit.vutbr.cz -p 1243 -S -n Roman
./client -p 1243 -NS -l xkunca04 -h eva.fit.vutbr.cz

./client -h eva.fit.vutbr.cz -p 1243 END
