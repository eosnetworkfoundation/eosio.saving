# `EOSIO Saving` distribution

## Quickstart

1. Setup distribution configurations
2. Transfer to distribute

```bash
# Setup distribution configurations
$ cleos push action eosio.saving setdistrib '[["eosio.grants", 8000], ["eosio.saving", 2000]]' -p eosio.saving

# Transfer to distribute
$ cleos transfer eosio eosio.saving "1.0000 EOS" "unallocated inflation"
```

## Edge cases

- Handle allocating to self (`eosio.saving`)
- Handle when not initialized state
- Handle when distribution accounts are empty

## Build

```
$ eosio-cpp eosio.saving.cpp -I include
```