#include <eosio.token/eosio.token.hpp>
#include "eosio.saving.hpp"

namespace eosio {

// [[eosio::action]]
// void saving::init( const extended_symbol sym ) {
//     // TO-DO
// }

[[eosio::action]]
void saving::setdistrib( const std::vector<distribute_account>& accounts ) {
    require_auth( get_self() );
    config_table _config_table{ get_self(), get_self().value };
    auto config = _config_table.get_or_default();

    if ( accounts.size() > 0 ) {
        int64_t remaining_percent = MAX_DISTRIBUTE_PERCENT;
        for ( const distribute_account dist_row : accounts ) {
            // check(dist_row.account != get_self(), "Cannot set account to " + get_self().to_string() );
            check(is_account(dist_row.account), "Account does not exist: " + dist_row.account.to_string() );
            check(0 < dist_row.percent, "Only positive percentages are allowed");
            check(dist_row.percent <= remaining_percent, "Total percentage exceeds 100%");
            remaining_percent -= dist_row.percent;
        }
        check(remaining_percent == 0, "Total percentage does not equal 100%");
    }
    // set accounts
    config.accounts = accounts;
    _distrib_singleton.set( config, get_self() );
}

[[eosio::action]]
void saving::claim(const name& claimer) {
    require_auth( claimer );

    auto& claimer = _claimers.get(claimer.value, "Not a valid distribution account");
    if (claimer.balance.amount != 0) {
        token::transfer_action transfer{ TOKEN_CONTRACT, { get_self(), "active"_n } };
        transfer_act.send( get_self(), claimer, claimer.balance, get_self().to_string() + " distribution claim" );
    }
    _claimers.erase(itr);
}

[[eosio::on_notify("*::transfer")]]
void saving::on_transfer(name from, name to, asset quantity, eosio::ignore<std::string> memo) {
    // tables
    config_table _config_table{ get_self(), get_self().value };

    // ignore transfer not directed to contract
    if (to != get_self()) return;

    // validate incoming transfer asset
    check( get_first_receiver() == TOKEN_CONTRACT, "Invalid contract");
    check( quantity.symbol == TOKEN_SYMBOL, "Invalid symbol");

    // ignore no-initialized contracts
    if ( !_config_table.exits() ) return;

    // distribute accounts
    const std::vector<distribute_account> accounts = _config_table.get().accounts;

    // ignore no distributions accounts
    if ( accounts.size() == 0 ) return;

    if (quantity.amount > 0) {
        asset remaining = quantity;

        for ( const distribute_account dist_row : accounts ) {
            if ( remaining.amount == 0 ) break;
            asset dist_amount = quantity * dist_row.percent / MAX_DISTRIBUTE_PERCENT;
            if ( remaining < dist_amount ) dist_amount = remaining;

            // add to table
            auto itr = _claimers.find(dist_row.account.value);
            if (itr == _claimers.end()) {
                // create record
                _claimers.emplace(get_self(), [&]( auto& row) {
                    row.account = acct.account;
                    row.balance = dist_amount;
                });
            } else {
                // update balance
                _claimers.modify(itr, get_self(), [&]( auto& row ){
                    row.balance += dist_amount;
                });
            }
            remaining -= dist_amount;
        }
    }
}

} // eosio