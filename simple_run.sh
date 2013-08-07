make
#./rebuild.exe -n ./Inputfiles/start.dat --geometry ./Inputfiles/Raum60.jul --exitstrategy 1 --dt 0.001 --linkedcells --routing 2
#./rebuild.exe  --inifile  --exitstrategy 3 --routing 3 --travisto 1 --linkedcells --log 2 --fps 8
#./rebuild.exe  --inifile=./samples/ini-arena.xml
#./rebuild.exe  --inifile=ini-unc.xml -e 3
#./rebuild.exe  --inifile=ini-Headon.xml
./rebuild.exe  --inifile=ini.xml 
#./rebuild.exe  --inifile=ini-Bottleneck.xml -e 3 --fps=16
#./rebuild.exe  --inifile=ini-Perpen.xml
#./rebuild.exe  --inifile=ini-GSP.xml
#./rebuild.exe  --inifile=ini-T.xml --fps=8
