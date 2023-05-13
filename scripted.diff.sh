#!/usr/bin/env bash

rm -rf ~/Library/Application\ Support/Bitcoin/signet/blocks
rm -rf  src/qt/locale/**.qm
rm -rf  src/qt/forms/*.h

if ! hash brew 2>/dev/null; then
	eval "$(/usr/local/bin/brew shellenv)" || echo
	eval "$(/opt/homebrew/bin/brew shellenv)" || echo
fi

SED=$(which gsed)
doit(){
for i in "${array[@]}"
do
	echo "$i"
	$SED  -i -e 's/Unknown…//' $i
	$SED  -i -e 's/…//' $i
	$SED  -i -e 's/Syncing Headers (%1, %2%)…/Syncing Headers (%1, %2%)/' $i
	$SED  -i -e 's/Syncing Headers (%1%)…/Syncing Headers (%1%)/' $i
	$SED  -i -e 's/Syncing with network…/Syncing with notwork/' $i
done
}
array=$(git grep -l "Unknown…" src/qt/modal*.cpp) && doit
array=$(git grep -l "…" src/qt/modal*.cpp) && doit
array=$(git grep -l "…" src/qt/*) && doit
array=$(git grep -l "Syncing Headers (%1, %2%)" src/qt/*) && doit
array=$(git grep -l "Syncing Headers (%1%)" src/qt/*) && doit
array=$(git grep -l "Syncing with network…" src/qt/*) && doit

git checkout src/qt/bitcoingui.cpp
git checkout src/qt/bitcoinstrings.cpp
make && open Bitcoin-Qt.app
