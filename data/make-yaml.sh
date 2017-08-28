#! /bin/bash

mkdir -p /mnt/export

rm /mnt/export/*

for f in /mnt/data/results/*.pgraph
do
	/mnt/data/make-yaml.rb ${f} /mnt/export/$(basename ${f}).yaml
done

eval $(/mnt/data/gather-all.sh)

echo "X" >> count_file.txt
