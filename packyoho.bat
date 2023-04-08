dir yohoplay*.prg
copy data\*.bin .
del index.bin
ren sa_index.bin index.bin
c:\app\vice\bin\c1541 -format yoho,id d64 yoho.d64 -write yohoplay.prg -write adv01.bin -write adv02.bin -write adv03.bin -write adv04.bin -write adv05.bin -write adv06.bin -write adv07.bin -write adv08.bin -write adv09.bin -write adv10.bin -write adv11.bin -write adv12.bin -write adv13.bin -write adv14a.bin -write adv14b.bin -write quest1.bin -write quest2.bin -write sampler1.bin -write index.bin
