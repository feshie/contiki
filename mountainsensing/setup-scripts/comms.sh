#!/bin/bash

echo "--- Comms Config ---"
IPV6PREFIX="2a01:348:24b:2:c30c::"

if [ "$#" -eq 0 ];then
        echo "Enter full Node Address"
        read node
else
        node=$IPV6PREFIX$1
fi
echo setting coms on $node

echo "Enter POST Interval (Sample count)"
read interval

#echo "Enter POST Proxy Address (No ::)"
#echo "setting Deployment POST proxy = 2a01:348:24b:2:0:0:0:1"
#echo "Weird private one =  aaaa:0:0:0:0:0:0:1" 
#read proxy
proxy=2a01:348:24b:2:0:0:0:1

a=`echo "$proxy" | cut -d ':' -f 1`
b=`echo "$proxy" | cut -d ':' -f 2`
c=`echo "$proxy" | cut -d ':' -f 3`
d=`echo "$proxy" | cut -d ':' -f 4`
e=`echo "$proxy" | cut -d ':' -f 5`
f=`echo "$proxy" | cut -d ':' -f 6`
g=`echo "$proxy" | cut -d ':' -f 7`
h=`echo "$proxy" | cut -d ':' -f 8`

echo "setting POST Proxy Port 8081"
port=8081
#read port

if wget -q  -O- 'http://['$node']/comsub?interval='$interval'&port='$port'&a='$a'&b='$b'&c='$c'&d='$d'&e='$e'&f='$f'&g='$g'&h='$h'&submit=Submit'
then
	echo "Successfully configured comms"
fi
