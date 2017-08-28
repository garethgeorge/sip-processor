#! /bin/bash

printf "curl -X POST"
for f in /mnt/export/*.yaml ; do
  printf " -F upload[]=@${f} "
done
printf " api:3030/update "
