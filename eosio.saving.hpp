#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>

#include <optional>
#include <string>

using namespace eosio;
using namespace std;

namespace eosio {

    static constexpr int64_t MAX_DISTRIBUTE_PERCENT = 100 * eosiosystem::inflation_precision; // 100%
    static constexpr int64_t SYSTEM_CORE_SYMBOL = symbol{"EOS", 4};eosiosystem::system_contract::get_core_symbol()

    struct distribute_account {
        name        account;
        uint16_t    percent;
    };

    class [[eosio::contract("eosio.saving")]] saving : public eosio::contract {
    public:
        using contract::contract;

        /**
         * ## TABLE `config`
         *
         * - `{distribute_account} accounts` - configuration accounts (ex: [ {"account": "eosio.grants", "percent": 10000} ])
         * - `{extended_symbol} sym` - extended symbol (ex: {"contract": "eosio.token", "symbol": "4,EOS"})
         *
         * ### example
         *
         * ```json
         * {
         *   "accounts": [
         *     {"account": "eosio.grants", "percent": 10000}
         *   ]
         *   "sym": {"contract": "eosio.token", "symbol": "4,EOS"}
         * }
         * ```
         */
        struct [[eosio::table("config")]] config_row {
            std::vector<distribute_account>     accounts
            extended_symbol                     sym;
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
        struct [[eosio::table("claimers")]] claimers_row {
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
        [[eosio::action]]
        void setdistrib( const std::vector<distribute_account>& accounts );

        [[eosio::action]]
        void init( const extended_symbol sym );

        /**
         * Claim tokens that have been marked for distribution.
         *
         * @pre `claimer` has tokens to claim
         *
         * @post row in `claimers` table will be erased
         * */
        [[eosio::action]]
        void claim(const name& claimer);

        /**
         * Action that will be called when eosio.token::transfer is performed.
         **/
        [[eosio::on_notify("eosio.token::transfer")]]
        void on_transfer(name from, name to, asset quantity, eosio::ignore<std::string> memo);

    }; // saving
} // eosio