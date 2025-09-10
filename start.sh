#!/bin/bash

docker compose build
docker compose up -d

docker cp $(docker compose ps -q rest-server):/export/hfml.json ./js/parser/