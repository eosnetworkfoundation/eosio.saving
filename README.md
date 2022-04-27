# `EOSIO Saving` distribution

## Quickstart

1. Setup distribution configurations
2. Transfer to distribute

```bash
# Setup distribution configurations
$ cleos push action eosio.saving setdistrib '[["eosio.grants", 10000]]' -p eosio.saving

# Transfer to distribute
$ cleos push action eosio.token transfer '["myaccount", "eosio.saving", "1.0000 EOS", "deposit"]' -p myaccount
```

## Edge cases

- Handle allocating to self (`eosio.saving`)
- Handle when not initialized state
- Handle when distribution accounts are empty

## Build

```
$ eosio-cpp eosio.saving.cpp -I include
```