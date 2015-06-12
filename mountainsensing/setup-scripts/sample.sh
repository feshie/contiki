#!/bin/bash

echo "--- Sample Config ---"

IPV6PREFIX="2a01:348:24b:2:c30c::"

if [ "$#" -eq 0 ];then
        echo "Enter Node Address"
        read node
else
        node=$IPV6PREFIX$1
fi
echo setting sampling settings on $node

echo "Enter Sample Interval (Seconds)"
read interval

echo "Enter . Seperated AVR IDs (Like 1.26.239)"
read avrids

echo "Does this node have a rain sensor? [y/N]"
read rain

echo "Does this node have a sensor for ADC1? [y/N]"
read adc1

echo "Does this node have a sensor for ADC2? [y/N]"
read adc2

if wget -q -O- 'http://['$node']/sensub?sample='$interval'&AVR='$avrids'&rain='$rain'&adc1='$adc1'&adc2='$adc2'&submit=Submit'
then
	echo "Successfully configured sample"
fi
