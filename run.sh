#!/bin/bash
export HS_USER="usernaam"
export HS_PASS="paswoord"

./home-security --user="$HS_USER" --password="$HS_PASS" --threshold=0.95 --overlay none
