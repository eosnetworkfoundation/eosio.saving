#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>

#include <string>

using namespace eosio;

namespace eosio {

    static constexpr int64_t MAX_DISTRIBUTE_PERCENT = 100'00; // 100%
    static constexpr symbol TOKEN_SYMBOL = symbol{"EOS", 4};
    static constexpr name TOKEN_CONTRACT = "eosio.token"_n;

    struct distribute_account {
        name        account;
        uint16_t    percent;
    };

    CONTRACT saving : public eosio::contract {
    public:
        using contract::contract;

        /**
         * ## TABLE `config`
         *
         * - `{distribute_account} accounts` - configuration accounts (ex: [ {"account": "eosio.grants", "percent": 10000} ])
         *
         * ### example
         *
         * ```json
         * {
         *   "accounts": [
         *     {"account": "eosio.grants", "percent": 10000}
         *   ]
         * }
         * ```
         */
        TABLE config_row {
            std::vector<distribute_account>     accounts;
        };
        typedef eosio::singleton< "config"_n, config_row > config_table;

        /**
         * ## TABLE `claimers`
         *
         * ### params
         *
         * - `{name} account` - account that can claim the distribution
         * - `{asset} balance` - current balance of the claimant account
         *
         * ### example
         *
         * ```json
         * {
         *     "account": "myaccount",
         *     "balance": "100.0000 EOS"
         * }
         * ```
         */
        TABLE claimers_row {
            name        account;
            asset       balance;

            uint64_t    primary_key() const { return account.value; }
        };
        typedef eosio::multi_index< "claimers"_n, claimers_row> claimers_table;

        /**
         * Set the accounts and their percentage of the distributed tokens.
         *
         * @pre accounts exist
         * @pre percentages add up to 100%
         *
         * @post any tokens sent to the contract account will be distributed based on the percentage
         * @post if `accounts` is an empty array no tokens will be distributed
         * */
        ACTION setdistrib( const std::vector<distribute_account>& accounts );

        /**
         * Claim tokens that have been marked for distribution.
         *
         * @pre `claimer` has tokens to claim
         *
         * @post row in `claimers` table will be erased
         * */
        ACTION claim( const name& claimer );

        /**
         * Action that will be called when transfer is performed.
         **/
        [[eosio::on_notify("*::transfer")]]
        void on_transfer( const name& from, const name& to, const asset& quantity, const string& memo );

    }; // saving
} // eosio