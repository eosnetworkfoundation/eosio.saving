import { Name, Asset, TimePointSec } from "@greymass/eosio";
import { Blockchain } from "@proton/vert"

// Vert EOS VM
const blockchain = new Blockchain()

// contracts
const contracts = {
  saving: blockchain.createContract('eosio.saving', 'eosio.saving', true),
  token: {
    EOS: blockchain.createContract('eosio.token', 'external/eosio.token/eosio.token'),
  },
}

// accounts
const accounts = blockchain.createAccounts('eosio', "eosio.grants");

interface Config {
  accounts: DistributeAccount[];
}

interface DistributeAccount {
  account: string;
  percent: number;
};

interface Claimers {
  account: string;
  balance: string;
}

const getConfig = (): Config => {
  const scope = Name.from('eosio.saving').value.value;
  return contracts.saving.tables.config(scope).getTableRows()[0];
}

const getBalance = ( account: string, symcode = "EOS" ): number => {
  const contract = (contracts.token as any)[symcode];
  const scope = Name.from(account).value.value;
  const primaryKey = Asset.SymbolCode.from(symcode).value.value;
  const result = contract.tables.accounts(scope).getTableRow(primaryKey);
  if ( result?.balance ) return Asset.from( result.balance ).value;
  return 0;
}

const getClaimer = ( account: string ): Claimers => {
  const scope = Name.from('eosio.saving').value.value;
  const primary_key = Name.from(account).value.value;
  return contracts.saving.tables.claimers(scope).getTableRow(primary_key)
}

// one-time setup
beforeAll(async () => {
  blockchain.setTime(TimePointSec.from(new Date()));

  // create EOS token
  await contracts.token.EOS.actions.create(["eosio", "10000000000.0000 EOS"]).send();
  await contracts.token.EOS.actions.issue(["eosio", "10000000000.0000 EOS", "init"]).send("eosio@active");
  await contracts.token.EOS.actions.transfer(["eosio", "eosio.saving", "100000.0000 EOS", "init"]).send("eosio@active");
});

describe('eosio.saving', () => {
  it("config::setdistrib", async () => {
    const accounts = [{account: "eosio.grants", percent: 8000}, {account: "eosio.saving", percent: 2000}];
    await contracts.saving.actions.setdistrib([accounts]).send();
    const config = getConfig();
    expect(config.accounts).toStrictEqual(accounts);
  });

  it("config::transfer", async () => {
    await contracts.token.EOS.actions.transfer(["eosio", "eosio.saving", "100.0000 EOS", "unallocated inflation"]).send("eosio@active");
    expect(getClaimer("eosio.grants").balance).toBe("80.0000 EOS");
  });
});

/**
 * Expect a promise to throw an error with a specific message.
 * @param promise - The promise to await.
 * @param {string} errorMsg - The error message that we expect to see.
 */
 const expectToThrow = async (promise: Promise<any>, errorMsg?: string) => {
  try {
    await promise
    expect(true).toBeFalsy();
  } catch (e: any) {
    if ( errorMsg ) expect(e.message).toMatch(errorMsg)
    else expect(false).toBeFalsy()
  }
}