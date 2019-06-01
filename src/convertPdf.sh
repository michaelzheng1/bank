#!/bin/bash
rm design.pdf
enscript -p design.ps design.txt
ps2pdf design.ps design.pdf
rm design.ps
rm convertPdf.sh~
