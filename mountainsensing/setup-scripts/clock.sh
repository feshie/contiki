#!/bin/bash

IPV6PREFIX="2a01:348:24b:2:c30c::"
echo "--- Clock Config ---"

if [ "$#" -eq 0 ];then
	echo "Enter Node Address"
	read node
else
	node=$IPV6PREFIX$1
fi
echo setting RTC on $node

year=`date +"%Y"`
month=`date +"%m"`
day=`date +"%d"`
hour=`date +"%H"`
min=`date +"%M"`
sec=`date +"%S"`

if wget -q -O- 'http://['$node']/clock?y='$year'&mo='$month'&d='$day'&h='$hour'&mi='$min'&s='$sec'&submit=Submit'
then
	echo "Successfully configured clock"
fi
