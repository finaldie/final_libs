#!/bin/bash

cd logs;

cat `ls -rt -1 | grep async_multithread_debug` | grep -oP "aid=\d+" | awk -F '=' '{print $2}' | awk 'BEGIN{error=0; last=0} {if ($1 != last + 1) {error +=1; printf "error: last=%d, curr=%d\n", last, $1} else {last = $1}} END{printf "error=%d\n", error}' | grep error=0
