#!/bin/bash

echo "compiling... [eosio.saving]"
blanc++ eosio.saving.cpp -I include -I ./external
shasum -a 256 eosio.saving.wasm
