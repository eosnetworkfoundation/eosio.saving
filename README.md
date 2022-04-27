# `EOSIO Saving` distribution

## Quickstart

1. Initialize contract
2. Setup configurations
3. Transfer to distribute

```bash
# Initialize contract
$ cleos push action eosio.saving init '[["eosio.token", "4,EOS"]]' -p eosio.saving

# Setup configurations
$ cleos push action eosio.saving setdistrib '[["eosio.grants", 10000]]' -p eosio.saving

# Transfer to distribute
$ cleos push action eosio.token transfer '["myaccount", "eosio.saving", "1.0000 EOS", "deposit"]' -p myaccount
```
