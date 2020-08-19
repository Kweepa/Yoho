rem \app\pucrunch\pucrunch -c20 yohoplay_unc.prg yohoplay.prg
rem \app\exomizer\win32\exomizer sfx basic -t52 -oyohoplay.prg yohoplay_unc.prg
dir yohoplay*.prg
copy data\*.bin .
del index.bin
ren bh_index.bin index.bin
c:\app\vice\c1541 -format howay,id d64 howay.d64 -write yohoplay.prg -write baton.bin -write timemachine.bin -write arrow1.bin -write arrow2.bin -write pulsar7.bin -write circus.bin -write feasibility.bin -write akyrz.bin -write perseus.bin -write tenlittle.bin -write waxworks.bin -write index.bin
