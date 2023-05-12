#!/usr/bin/env bash
rm -rf  src/qt/locale/bitcoin_*.qm
#ls      src/qt/locale/
rm -rf  src/qt/forms/*.h
#ls      src/qt/forms/

#Syncing Headers (%1, %2%)…
git grep "Syncing Headers (%1, %2%)…" >  test.txt && cat test.txt
git grep "Syncing Headers (%1, %2%)…" ./src/qt > test2.txt && cat test2.txt
git grep    "Syncing Headers (%1, %2%)…" ./src/qt/locale/**.ts
sed -e 's/Syncing Headers (%1, %2%)…/Syncing Headers (%1, %2%)/g' $(git grep -l "Syncing Headers (%1, %2%)…" ./src/qt/forms/*)
#Syncing Headers (%1%)…
#git grep "Syncing Headers (%1%)…" >  test.txt && cat test.txt
#git grep "Syncing Headers (%1%)…" ./src/qt > test2.txt && cat test2.txt
#git grep    "Syncing Headers (%1%)…" ./src/qt/*
#sed -I 's/Syncing Headers (%1%)…/Syncing Headers (%1%)/g' $(git grep -l "Syncing Headers (%1%)…" ./src/qt/*)

