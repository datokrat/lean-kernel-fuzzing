#!/bin/sh

rm -r input/
cp -r ../lean4export/ExportedCorpus/ input/
cp ../lean4export/prelude.elean .
cp ../lean4export/strings .
