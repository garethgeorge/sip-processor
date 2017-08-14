#! /bin/bash

mkdir -p /mnt/export

rm /mnt/export/*

for f in /mnt/data/results/*.pgraph
do
	/mnt/data/make-yaml.rb ${f} /mnt/export/$(basename ${f}).yaml
done

date --rfc-3339="seconds" > /mnt/export/last_update.txt
echo $1 >> /mnt/export/last_update.txt

curl -X POST sip-api:3000/update

echo "X" >> count_file.txt
