#!/usr/bin/env bash

rm -rf ~/Library/Application\ Support/Bitcoin/signet/blocks
rm -rf  src/qt/locale/**.qm
rm -rf  src/qt/forms/*.h
rm -rf  src/qt/moc_modaloverlay.cpp
rm -rf  dist/Bitcoin-Qt.app
rm -rf  Bitcoin-Qt.app


if ! hash brew 2>/dev/null; then
	eval "$(/usr/local/bin/brew shellenv)" || echo
	eval "$(/opt/homebrew/bin/brew shellenv)" || echo
fi

SED=$(which gsed)
doit(){
for i in "${array[@]}"
do
	echo "$i"
	$SED  -i -e 's/$j//' $i
done
}

#…
#src/qt/forms/receiverequestdialog.ui
#array=$(git grep -l "…" src/qt/modal*.cpp) && doit
#array=$(git grep -l "…" src/qt/*) && doit

strings=("…" "Unknown…" "Syncing Headers (%1, %2%)…" "Syncing Headers (%1%)…" "Syncing with network…" "&amp;Options…")

for j in "${strings[@]}"
do
	echo "$j"
	array=$(git grep -l $j src/qt/*) && doit
	array=$(git grep -l $j src/qt/forms/*) && doit
	array=$(git grep -l $j src/qt/locale/*) && doit
	array=$(git grep -l $j src/qt/modal*.cpp) && doit
done

#git checkout src/qt/forms/intro.ui
#git checkout src/qt/bitcoin.cpp
#git checkout src/qt/bitcoingui.cpp
#git checkout src/qt/bitcoinstrings.cpp

git diff
make deploy
open ./dist/Bitcoin-Qt.app
