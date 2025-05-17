#!/bin/sh

if [ "$#" -ne 3 ]; then
  echo "Usage: $0 REMOTE_URL PICO_VERSION FIRMWARE.elf"
  echo "    PICO_VERSION can be 'rp2040' or 'rp2350'"
  exit 1
fi

curl -X POST -H "Content-Type: application/octet-stream" -d "{ \"cpu\": \"$2\", \"firmware\": \"$(base64 -i $3 | tr -d '\n')\" }" http://$1:8376|jq -r '.'