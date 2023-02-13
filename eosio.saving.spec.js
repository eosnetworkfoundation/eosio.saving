const { Name, Asset, TimePointSec } = require("@greymass/eosio");
const { Blockchain } = require("@proton/vert");

// Vert EOS VM
const blockchain = new Blockchain()
blockchain.enableStorageDeltas();

// contracts
const contracts = {
  saving: blockchain.createContract('eosio.saving', 'eosio.saving', true),
  token: {
    EOS: blockchain.createContract('eosio.token', 'include/eosio.token/eosio.token'),
  },
  fake: {
    EOS: blockchain.createContract('fake.token', 'include/eosio.token/eosio.token'),
  },
}

// accounts
const accounts = blockchain.createAccounts('eosio', "eosio.grants", "eosio.other", "eosio.second");

const getConfig = () => {
  const scope = Name.from('eosio.saving').value.value;
  return contracts.saving.tables.config(scope).getTableRows()[0];
}

const getBalance = ( account, symcode = "EOS" ) => {
  const contract = contracts.token[symcode];
  const scope = Name.from(account).value.value;
  const primaryKey = Asset.SymbolCode.from(symcode).value.value;
  const result = contract.tables.accounts(scope).getTableRow(primaryKey);
  if ( result?.balance ) return Asset.from( result.balance ).units.toNumber();
  return 0;
}

const getClaimer = ( account ) => {
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

  // create fake EOS token
  await contracts.fake.EOS.actions.create(["eosio", "10000000000.0000 EOS"]).send();
  await contracts.fake.EOS.actions.issue(["eosio", "10000000000.0000 EOS", "init"]).send("eosio@active");
});

describe('eosio.saving', () => {
  it("transfer pre-config", async () => {
    await contracts.token.EOS.actions.transfer(["eosio", "eosio.saving", "100000.0000 EOS", "init"]).send("eosio@active");
    await contracts.token.EOS.actions.transfer(["eosio.saving", "eosio", "100000.0000 EOS", "init"]).send("eosio.saving@active");
  });

  it("config::setdistrib (100%)", async () => {
    const accounts = [{account: "eosio.grants", percent: 10000}];
    await contracts.saving.actions.setdistrib([accounts]).send();
    expect(getConfig().accounts).toStrictEqual(accounts);
    blockchain.printStorageDeltas();
  });

  it("allocate", async () => {
    await contracts.token.EOS.actions.transfer(["eosio", "eosio.saving", "100.0000 EOS", "unallocated inflation"]).send("eosio@active");
    expect(getClaimer("eosio.grants").balance).toBe("100.0000 EOS");
    blockchain.printStorageDeltas();
  });

  it("claim", async () => {
    await contracts.saving.actions.claim(["eosio.grants"]).send("eosio.grants@active");
    expect(getBalance("eosio.grants", "EOS")).toBe(1000000);
    blockchain.printStorageDeltas();
  });

  it("setdistrib (50/30/20)", async () => {
    const accounts = [{account: "eosio.grants", percent: 5000}, {account: "eosio.other", percent: 3000}, {account: "eosio.second", percent: 2000}];
    await contracts.saving.actions.setdistrib([accounts]).send();
    expect(getConfig().accounts).toStrictEqual(accounts);

    // transfer
    await contracts.token.EOS.actions.transfer(["eosio", "eosio.saving", "100.0000 EOS", "unallocated inflation"]).send("eosio@active");
    expect(getClaimer("eosio.grants").balance).toBe("50.0000 EOS");
    expect(getClaimer("eosio.other").balance).toBe("30.0000 EOS");
    expect(getClaimer("eosio.second").balance).toBe("20.0000 EOS");

    // claim
    await contracts.saving.actions.claim(["eosio.grants"]).send("eosio.grants@active");
    await contracts.saving.actions.claim(["eosio.other"]).send("eosio.other@active");
    await contracts.saving.actions.claim(["eosio.second"]).send("eosio.second@active");
    expect(getBalance("eosio.grants", "EOS")).toBe(1500000); // 100.0000 EOS + 50.0000 EOS
    expect(getBalance("eosio.other", "EOS")).toBe(300000); // 30.0000 EOS;
    expect(getBalance("eosio.second", "EOS")).toBe(200000); // 20.0000 EOS;
    expect(getBalance("eosio.saving", "EOS")).toBe(0); // 0.0000 EOS
  });

  it("small transfer", async () => {
    await contracts.token.EOS.actions.transfer(["eosio", "eosio.saving", "0.0010 EOS", "unallocated inflation"]).send("eosio@active");
    expect(getClaimer("eosio.grants").balance).toBe("0.0005 EOS");
    expect(getClaimer("eosio.other").balance).toBe("0.0003 EOS");
    expect(getClaimer("eosio.second").balance).toBe("0.0002 EOS");
  });

  it("smallest transfer", async () => {
    await contracts.token.EOS.actions.transfer(["eosio", "eosio.saving", "0.0004 EOS", "unallocated inflation"]).send("eosio@active");
    expect(getClaimer("eosio.grants").balance).toBe("0.0007 EOS"); // +0.0002 EOS
    expect(getClaimer("eosio.other").balance).toBe("0.0004 EOS"); // +0.0001 (30% = 1.8 )
    expect(getClaimer("eosio.second").balance).toBe("0.0002 EOS"); // no change (20% = 0.8 )
  });

  it("setdistrib (empty)", async () => {
    await contracts.saving.actions.setdistrib([[]]).send();
    expect(getConfig().accounts).toStrictEqual([]);
  });

  // ERRORS
  it("error::setdistrib (50)", async () => {
    const accounts = [{account: "eosio.grants", percent: 5000}];
    const action = contracts.saving.actions.setdistrib([accounts]).send();
    await expectToThrow(action, "Total percentage does not equal 100%");
  });

  it("error::setdistrib (0)", async () => {
    const accounts = [{account: "eosio.grants", percent: 0}];
    const action = contracts.saving.actions.setdistrib([accounts]).send();
    await expectToThrow(action, "Only positive percentages are allowed");
  });

  it("error::setdistrib (null account)", async () => {
    const accounts = [{account: "null", percent: 10000}];
    const action = contracts.saving.actions.setdistrib([accounts]).send();
    await expectToThrow(action, "Account does not exist");
  });

  it("error::setdistrib (25/25/25)", async () => {
    const accounts =["eosio", "eosio.grants", "eosio.saving"].map(account => { return {account, percent: 2500}});
    const action = contracts.saving.actions.setdistrib([accounts]).send();
    await expectToThrow(action, "Total percentage does not equal 100%");
  });

  it("error::setdistrib (50/100)", async () => {
    const accounts = [{account: "eosio.grants", percent: 5000}, {account: "eosio.saving", percent: 10000}];
    const action = contracts.saving.actions.setdistrib([accounts]).send();
    await expectToThrow(action, "Total percentage exceeds 100%");
  });

  it("fake transfer", async () => {
    const action = contracts.fake.EOS.actions.transfer(["eosio", "eosio.saving", "100.0000 EOS", "fake unallocated inflation"]).send("eosio@active");
    await expectToThrow(action, "Invalid contract");
  });
});

/**
 * Expect a promise to throw an error with a specific message.
 * @param promise - The promise to await.
 * @param {string} errorMsg - The error message that we expect to see.
 */
 const expectToThrow = async (promise, errorMsg) => {
  try {
    await promise
    expect(true).toBeFalsy();
  } catch (e) {
    if ( errorMsg ) expect(e.message).toMatch(errorMsg)
    else expect(false).toBeFalsy()
  }
}