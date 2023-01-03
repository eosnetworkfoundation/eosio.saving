# `EOSIO Saving` distribution
[![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/eosnetworkfoundation/eosio.saving/blob/main/LICENSE)
[![Antelope CDT](https://github.com/eosnetworkfoundation/eosio.saving/actions/workflows/release.yml/badge.svg)](https://github.com/eosnetworkfoundation/eosio.saving/actions/workflows/release.yml)
[![Blanc++ Vert](https://github.com/eosnetworkfoundation/eosio.saving/actions/workflows/tests.yml/badge.svg)](https://github.com/eosnetworkfoundation/eosio.saving/actions/workflows/tests.yml)

## Quickstart

1. Setup distribution configurations
2. Transfer to distribute

```bash
# Setup distribution configurations
$ cleos push action eosio.saving setdistrib '[["eosio.grants", 8000], ["eosio.saving", 2000]]' -p eosio.saving

# Transfer to distribute
$ cleos transfer eosio eosio.saving "1.0000 EOS" "unallocated inflation"

# Claim allocation
$ cleos push action eosio.saving claim '["eosio.grants"]' -p eosio.grants
# //=> transfer 0.8000 EOS from `eosio.saving` to `eosio.grants`
```

## Edge cases

- Handle allocating to self (`eosio.saving`)
- Handle when not initialized state
- Handle when distribution accounts are empty


## Build

```bash
# using Antelope CDT
$ cdt-cpp eosio.saving.cpp -I include
# using Blanc++
$ blanc++ eosio.saving.cpp -I include
```

## SHA256 Checksum

**CDT**
- [Blanc++ `v0.12.0`](https://github.com/haderech/blanc/releases/tag/0.12.1)
- [Antelope CDT `v3.1.0`](https://github.com/AntelopeIO/cdt/releases/tag/v3.1.0)

```bash
$ git clone https://github.com/eosnetworkfoundation/eosio.saving.git
$ cd eosio.saving
$ cdt-cpp eosio.saving.cpp
$ shasum -a 256 eosio.saving.wasm
fc0dc47848a5d69ccd99fc60f74aa08c68bd734e1f2bc4b4c99191b99b39cffc  eosio.saving.wasm
```

**EOS Mainnet**

```bash
$ curl -X POST https://eos.greymass.com/v1/chain/get_code -d '{"account_name":"eosio.saving"}' | jq .code_hash
fc0dc47848a5d69ccd99fc60f74aa08c68bd734e1f2bc4b4c99191b99b39cffc
```