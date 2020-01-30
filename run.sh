#!/bin/bash
export CAM_USER="usernaam"
export CAM_PASS="paswoord"

./home-security --user="$CAM_USER" --password="$CAM_PASS" --threshold=0.95 --overlay none
