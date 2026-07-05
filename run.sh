#!/bin/bash

cd "$(dirname "$0")"
mame vectrex -cart main.bin -skip_gameinfo
