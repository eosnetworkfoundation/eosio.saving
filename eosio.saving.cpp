#include <eosio.token/eosio.token.hpp>
#include "eosio.saving.hpp"

[[eosio::action]]
void saving::setdistrib( const std::vector<distribute_account>& accounts ) {
    require_auth( get_self() );
    config_table _config{ get_self(), get_self().value };
    auto config = _config.get_or_default();

    if ( accounts.size() > 0 ) {
        int64_t remaining_percent = MAX_DISTRIBUTE_PERCENT;
        for ( const distribute_account dist_row : accounts ) {
            check(is_account(dist_row.account), "Account does not exist: " + dist_row.account.to_string() );
            check(0 < dist_row.percent, "Only positive percentages are allowed");
            check(dist_row.percent <= remaining_percent, "Total percentage exceeds 100%");
            remaining_percent -= dist_row.percent;
        }
        check(remaining_percent == 0, "Total percentage does not equal 100%");
    }
    // set accounts
    config.accounts = accounts;
    _config.set( config, get_self() );
}

[[eosio::action]]
void saving::claim(const name& claimer) {
    require_auth( claimer );
    claimers_table _claimers{ get_self(), get_self().value };

    auto& itr = _claimers.get(claimer.value, "Distribution account does not exists");
    if (itr.balance.amount != 0) {
        eosio::token::transfer_action transfer{ TOKEN_CONTRACT, { get_self(), "active"_n } };
        transfer.send( get_self(), claimer, itr.balance, get_self().to_string() + " distribution claim" );
    }
    _claimers.erase(itr);
}

[[eosio::on_notify("*::transfer")]]
void saving::on_transfer( const name& from, const name& to, const asset& quantity, const string& memo ) {
    // tables
    config_table _config{ get_self(), get_self().value };
    claimers_table _claimers{ get_self(), get_self().value };

    // ignore transfer not directed to contract
    if (to != get_self()) return;

    // validate incoming transfer asset
    check( get_first_receiver() == TOKEN_CONTRACT, "Invalid contract");
    check( quantity.symbol == TOKEN_SYMBOL, "Invalid symbol");
    check( quantity.amount > 0, "Quantity must be positive");

    // ignore no-initialized contracts
    if ( !_config.exists() ) return;

    // distribute accounts
    const auto accounts = _config.get().accounts;

    // ignore no distributions accounts
    if ( accounts.size() == 0 ) return;

    asset remaining = quantity;
    for ( const distribute_account dist_row : accounts ) {
        if ( remaining.amount == 0 ) break;
        asset dist_amount = quantity * dist_row.percent / MAX_DISTRIBUTE_PERCENT;

        // handle distributing dust from last account
        if ( remaining < dist_amount ) dist_amount = remaining;
        remaining -= dist_amount;

        // do not update claimer balance if self (eosio.saving)
        if ( dist_row.account == get_self() ) continue;

        // ignore dust (0.0000 EOS) no need to create/update table
        if ( dist_amount.amount <= 0 ) continue;

        // add to table
        auto itr = _claimers.find(dist_row.account.value);
        if (itr == _claimers.end()) {
            // create initial balance
            _claimers.emplace(get_self(), [&]( auto& row) {
                row.account = dist_row.account;
                row.balance = dist_amount;
            });
        } else {
            // update existing balance
            _claimers.modify(itr, get_self(), [&]( auto& row ){
                row.balance += dist_amount;
            });
        }
    }
}
