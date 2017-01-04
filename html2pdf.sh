#!/bin/bash

for f in $PWD/*.html; do 
   echo $f
   ./url2pdf --url=file://${f} --autosave-path=$PWD --print-paginate=NO # --autosave-name=${f}.pdf
done