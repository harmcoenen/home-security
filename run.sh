#!/bin/bash

export HS_USER="usernaam"
export HS_PASS="paswoord"
export IP_ADDR=`ifconfig eth0 | grep "inet " | cut -d ' ' -f 10`

./home-security --ipaddr="$IP_ADDR" --user="$HS_USER" --password="$HS_PASS" --camera=/dev/video1 --threshold=0.65 --overlay none
