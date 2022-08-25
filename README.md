# `EOSIO Saving` distribution [![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/facebook/react/blob/main/LICENSE) [![EOSIO Smart Contract](https://github.com/eosnetworkfoundation/eosio.saving/actions/workflows/tests.yml/badge.svg)](https://github.com/eosnetworkfoundation/eosio.saving/actions/workflows/tests.yml)

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

```
$ eosio-cpp eosio.saving.cpp -I include
```