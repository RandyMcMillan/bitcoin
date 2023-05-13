#!/usr/bin/env bash

rm -rf  src/qt/locale/**.qm
rm -rf  src/qt/forms/*.h

doit(){
for i in "${array[@]}"
do
	echo "$i"
	gsed  -i -e 's/Syncing Headers (%1, %2%)…/Syncing Headers (%1, %2%)/' $i
	gsed  -i -e 's/Syncing Headers (%1%)…/Syncing Headers (%1%)/' $i
	gsed  -i -e 's/Syncing with network…/Syncing with notwork/' $i
	gsed  -i -e 's/Unknown…//' $i
	gsed  -i -e 's/Unknown //' $i
done
}
array=$(git grep -l "Syncing Headers (%1, %2%)" src/*) && doit
array=$(git grep -l "Syncing Headers (%1%)" src/*) && doit
array=$(git grep -l "Syncing with network…" src/*) && doit
array=$(git grep -l "Unknown…" src/*) && doit
array=$(git grep -l "Unknown " src/*) && doit
