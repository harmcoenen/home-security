#!/bin/bash
export HS_USER="usernaam"
export HS_PASS="paswoord"
export IP_ADDR=`ifconfig eth0 | grep "inet " | cut -d ' ' -f 10`

./home-security --ipaddr="$IP_ADDR" --user="$HS_USER" --password="$HS_PASS" --threshold=0.95 --overlay none
