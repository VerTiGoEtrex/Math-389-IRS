#!/bin/bash

success=""
failure=""

for (( i = $1; i < $2; i++ )); do
  ./bf.out $i 1>&- 2>&-
  if [ $? -ne 0 ]
  then
    echo "$i failed";
    failure+=" "$i;
  else
    echo "$i success";
    success+=" "$i;
  fi
done
echo "Success: $success";
echo "Failure: $failure";
