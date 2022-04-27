#include <eosio.token/eosio.token.hpp>
#include "eosio.saving.hpp"

namespace eosio {

[[eosio::action]]
void saving::init( const extended_symbol sym ) {
    // TO-DO
}

[[eosio::action]]
void saving::setdistrib( const std::vector<distribute_account>& accounts ) {
    require_auth( get_self() );

    if ( accounts.size() > 0 ) {
        int64_t remaining_percent = MAX_DISTRIBUTE_PERCENT;
        for ( const distribute_account row : accounts ) {
            check(row.account != get_self(), "Cannot set account to " + get_self().to_string() );
            check(is_account(row.account), "Account does not exist: " + row.account.to_string() );
            check(0 < row.percent, "Only positive percentages are allowed");
            check(row.percent <= remaining_percent, "Total percentage exceeds 100%");
            remaining_percent -= row.percent;
        }
        check(remaining_percent == 0, "Total percentage does not equal 100%");
    }
    // set accounts
    auto& accts = _distrib_state.accounts;
    accts = accounts;

    _distrib_singleton.set( _distrib_state, get_self() );
}

[[eosio::action]]
void saving::claim(const name& claimer) {
    require_auth(claimer);

    auto itr = _claimers.find(claimer.value);
    check(itr != _claimers.end(), "Not a valid distribution account");
    if (itr->balance.amount != 0) {
        token::transfer_action transfer_act{ system_contract::token_account, { get_self(), system_contract::active_permission } };
        transfer_act.send( get_self(), claimer, itr->balance, "distribution claim" );
    }
    _claimers.erase(itr);
}

[[eosio::on_notify("eosio.token::transfer")]]
void saving::on_transfer(name from, name to, asset quantity, eosio::ignore<std::string> memo) {
    if (to != get_self()) return;

    check( quantity.symbol == eosiosystem::system_contract::get_core_symbol(), "Invalid symbol");
    check( _distrib_state.accounts.size() > 0, "distribution accounts were not setup");

    if (quantity.amount > 0) {
        asset remaining = quantity;
        for ( const auto& acct : _distrib_state.accounts) {
            if ( remaining.amount == 0 ) break;
            const asset dist_amount = quantity * acct.percent / MAX_DISTRIBUTE_PERCENT;
            if ( remaining < dist_amount ) dist_amount = remaining;

            // add to table index
            auto citr = _claimers.find(acct.account.value);
            if (citr == _claimers.end()) {
                // create record
                _claimers.emplace(get_self(), [&]( auto& row) {
                    row.account = acct.account;
                    row.balance = dist_amount;
                });
            } else {
                // update balance
                _claimers.modify(citr, get_self(), [&]( auto& row ){
                    row.balance += dist_amount;
                });
            }
            remaining -= dist_amount;
        }
    }
}

} // eosio