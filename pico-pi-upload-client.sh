#!/bin/sh

if [ "$#" -ne 3 ]; then
  echo "Usage: $0 REMOTE_URL PICO_VERSION FIRMWARE.elf"
  echo "    PICO_VERSION can be 'rp2040' or 'rp2350'"
  exit 1
fi

printf "%s" "{ \"cpu\": \"$2\", \"firmware\": \"" > temp.json
base64 -i $3 | tr -d '\n' >> temp.json
echo '" }' >> temp.json
curl -X POST -H "Content-Type: application/octet-stream" -d @temp.json http://$1:8376|jq -r '.'
rm temp.json
