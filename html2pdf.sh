#!/bin/bash

if [ -z "$1" ]; then
for f in $PWD/*.html; do 
   echo $f
   echo ./url2pdf --url=file://${f/ /%20} --autosave-path=$PWD --print-paginate=NO --autosave-name=URL
done
else
   f=$1
   echo ./url2pdf --url=file://${f/ /%20} --autosave-path=$PWD --print-paginate=NO --autosave-name=URL
   ./url2pdf --url=file://${f/ /%20} --autosave-path=$PWD --print-paginate=NO --autosave-name=URL
fi
