#!/usr/bin/env bash

if [[ $EUID -ne 0 ]]; then
    echo "This script must be run as root. Use: sudo $0"
    exit 1
fi

echo "Installing lkeb-artifactory certificate"
cp cmake/lkeb-artifactory-lumc-nl-chain.crt /usr/local/share/ca-certificates/
update-ca-certificates