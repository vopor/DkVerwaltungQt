#!/bin/bash

for f in $PWD/*.html; do 
   echo $f
   ./url2pdf --url=file://${f/ /%20} --autosave-path=$PWD --print-paginate=NO --autosave-name=URL
done