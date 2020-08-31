// Copyright (c) 2018-2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <interfaces/wallet.h>

#include <amount.h>
#include <interfaces/chain.h>
#include <interfaces/handler.h>
#include <policy/fees.h>
#include <primitives/transaction.h>
#include <rpc/server.h>
#include <script/standard.h>
#include <support/allocators/secure.h>
#include <sync.h>
#include <uint256.h>
#include <util/check.h>
#include <util/ref.h>
#include <util/system.h>
#include <util/ui_change_type.h>
#include <wallet/context.h>
#include <wallet/feebumper.h>
#include <wallet/fees.h>
#include <wallet/ismine.h>
#include <wallet/load.h>
#include <wallet/rpcwallet.h>
#include <wallet/wallet.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace interfaces {
namespace {

//! Construct wallet tx struct.
WalletTx MakeWalletTx(CWallet& wallet, const CWalletTx& wtx)
{
    LOCK(wallet.cs_wallet);
    WalletTx result;
    result.tx = wtx.tx;
    result.txin_is_mine.reserve(wtx.tx->vin.size());
    for (const auto& txin : wtx.tx->vin) {
        result.txin_is_mine.emplace_back(wallet.IsMine(txin));
    }
    result.txout_is_mine.reserve(wtx.tx->vout.size());
    result.txout_address.reserve(wtx.tx->vout.size());
    result.txout_address_is_mine.reserve(wtx.tx->vout.size());
    for (const auto& txout : wtx.tx->vout) {
        result.txout_is_mine.emplace_back(wallet.IsMine(txout));
        result.txout_address.emplace_back();
        result.txout_address_is_mine.emplace_back(ExtractDestination(txout.scriptPubKey, result.txout_address.back()) ?
                                                      wallet.IsMine(result.txout_address.back()) :
                                                      ISMINE_NO);
    }
    result.credit = wtx.GetCredit(ISMINE_ALL);
    result.debit = wtx.GetDebit(ISMINE_ALL);
    result.change = wtx.GetChange();
    result.time = wtx.GetTxTime();
    result.value_map = wtx.mapValue;
    result.is_coinbase = wtx.IsCoinBase();
    return result;
}

//! Construct wallet tx status struct.
WalletTxStatus MakeWalletTxStatus(CWallet& wallet, const CWalletTx& wtx)
{
    WalletTxStatus result;
    result.block_height = wtx.m_confirm.block_height > 0 ? wtx.m_confirm.block_height : std::numeric_limits<int>::max();
    result.blocks_to_maturity = wtx.GetBlocksToMaturity();
    result.depth_in_main_chain = wtx.GetDepthInMainChain();
    result.time_received = wtx.nTimeReceived;
    result.lock_time = wtx.tx->nLockTime;
    result.is_final = wallet.chain().checkFinalTx(*wtx.tx);
    result.is_trusted = wtx.IsTrusted();
    result.is_abandoned = wtx.isAbandoned();
    result.is_coinbase = wtx.IsCoinBase();
    result.is_in_main_chain = wtx.IsInMainChain();
    return result;
}

//! Construct wallet TxOut struct.
WalletTxOut MakeWalletTxOut(CWallet& wallet,
    const CWalletTx& wtx,
    int n,
    int depth) EXCLUSIVE_LOCKS_REQUIRED(wallet.cs_wallet)
{
    WalletTxOut result;
    result.txout = wtx.tx->vout[n];
    result.time = wtx.GetTxTime();
    result.depth_in_main_chain = depth;
    result.is_spent = wallet.IsSpent(wtx.GetHash(), n);
    return result;
}

class WalletClientImpl : public ChainClient
{
public:
    WalletClientImpl(Chain& chain, ArgsManager& args, std::vector<std::string> wallet_filenames)
        : m_wallet_filenames(std::move(wallet_filenames))
    {
        m_context.chain = &chain;
        m_context.args = &args;
    }
    void registerRpcs() override
    {
        for (const CRPCCommand& command : GetWalletRPCCommands()) {
            m_rpc_commands.emplace_back(command.category, command.name, [this, &command](const JSONRPCRequest& request, UniValue& result, bool last_handler) {
                return command.actor({request, m_context}, result, last_handler);
            }, command.argNames, command.unique_id);
            m_rpc_handlers.emplace_back(m_context.chain->handleRpc(m_rpc_commands.back()));
        }
    }
    bool verify() override { return VerifyWallets(*m_context.chain, m_wallet_filenames); }
    bool load() override { return LoadWallets(*m_context.chain, m_wallet_filenames); }
    void start(CScheduler& scheduler) override { return StartWallets(scheduler, *Assert(m_context.args)); }
    void flush() override { return FlushWallets(); }
    void stop() override { return StopWallets(); }
    void setMockTime(int64_t time) override { return SetMockTime(time); }
    std::vector<std::unique_ptr<Wallet>> getWallets() override
    {
        std::vector<std::unique_ptr<Wallet>> wallets;
        for (const auto& wallet : GetWallets()) {
            wallets.emplace_back(MakeWallet(wallet));
        }
        return wallets;
    }
    ~WalletClientImpl() override { UnloadWallets(); }

    WalletContext m_context;
    const std::vector<std::string> m_wallet_filenames;
    std::vector<std::unique_ptr<Handler>> m_rpc_handlers;
    std::list<CRPCCommand> m_rpc_commands;
};

} // namespace

WalletImpl::WalletImpl(const std::shared_ptr<CWallet>& wallet) : m_wallet(wallet) {}

bool WalletImpl::encryptWallet(const SecureString& wallet_passphrase)
{
    return m_wallet->EncryptWallet(wallet_passphrase);
}
bool WalletImpl::isCrypted() { return m_wallet->IsCrypted(); }
bool WalletImpl::lock() { return m_wallet->Lock(); }
bool WalletImpl::unlock(const SecureString& wallet_passphrase) { return m_wallet->Unlock(wallet_passphrase); }
bool WalletImpl::isLocked() { return m_wallet->IsLocked(); }
bool WalletImpl::changeWalletPassphrase(const SecureString& old_wallet_passphrase,
    const SecureString& new_wallet_passphrase) {
    return m_wallet->ChangeWalletPassphrase(old_wallet_passphrase, new_wallet_passphrase);
}
void WalletImpl::abortRescan() { m_wallet->AbortRescan(); }
bool WalletImpl::backupWallet(const std::string& filename) { return m_wallet->BackupWallet(filename); }
std::string WalletImpl::getWalletName() { return m_wallet->GetName(); }
bool WalletImpl::getNewDestination(const OutputType type, const std::string label, CTxDestination& dest)
{
    LOCK(m_wallet->cs_wallet);
    std::string error;
    return m_wallet->GetNewDestination(type, label, dest, error);
}
bool WalletImpl::getPubKey(const CScript& script, const CKeyID& address, CPubKey& pub_key)
{
    std::unique_ptr<SigningProvider> provider = m_wallet->GetSolvingProvider(script);
    if (provider) {
        return provider->GetPubKey(address, pub_key);
    }
    return false;
}
SigningResult WalletImpl::signMessage(const std::string& message, const PKHash& pkhash, std::string& str_sig)
{
    return m_wallet->SignMessage(message, pkhash, str_sig);
}
bool WalletImpl::isSpendable(const CTxDestination& dest)
{
    LOCK(m_wallet->cs_wallet);
    return m_wallet->IsMine(dest) & ISMINE_SPENDABLE;
}
bool WalletImpl::haveWatchOnly()
{
    auto spk_man = m_wallet->GetLegacyScriptPubKeyMan();
    if (spk_man) {
        return spk_man->HaveWatchOnly();
    }
    return false;
};
bool WalletImpl::setAddressBook(const CTxDestination& dest, const std::string& name, const std::string& purpose)
{
    return m_wallet->SetAddressBook(dest, name, purpose);
}
bool WalletImpl::delAddressBook(const CTxDestination& dest)
{
    return m_wallet->DelAddressBook(dest);
}
bool WalletImpl::getAddress(const CTxDestination& dest,
    std::string* name,
    isminetype* is_mine,
    std::string* purpose)
{
    LOCK(m_wallet->cs_wallet);
    auto it = m_wallet->m_address_book.find(dest);
    if (it == m_wallet->m_address_book.end() || it->second.IsChange()) {
        return false;
    }
    if (name) {
        *name = it->second.GetLabel();
    }
    if (is_mine) {
        *is_mine = m_wallet->IsMine(dest);
    }
    if (purpose) {
        *purpose = it->second.purpose;
    }
    return true;
}
std::vector<WalletAddress> WalletImpl::getAddresses()
{
    LOCK(m_wallet->cs_wallet);
    std::vector<WalletAddress> result;
    for (const auto& item : m_wallet->m_address_book) {
        if (item.second.IsChange()) continue;
        result.emplace_back(item.first, m_wallet->IsMine(item.first), item.second.GetLabel(), item.second.purpose);
    }
    return result;
}
bool WalletImpl::addDestData(const CTxDestination& dest, const std::string& key, const std::string& value)
{
    LOCK(m_wallet->cs_wallet);
    WalletBatch batch{m_wallet->GetDatabase()};
    return m_wallet->AddDestData(batch, dest, key, value);
}
bool WalletImpl::eraseDestData(const CTxDestination& dest, const std::string& key)
{
    LOCK(m_wallet->cs_wallet);
    WalletBatch batch{m_wallet->GetDatabase()};
    return m_wallet->EraseDestData(batch, dest, key);
}
std::vector<std::string> WalletImpl::getDestValues(const std::string& prefix)
{
    LOCK(m_wallet->cs_wallet);
    return m_wallet->GetDestValues(prefix);
}
void WalletImpl::lockCoin(const COutPoint& output)
{
    LOCK(m_wallet->cs_wallet);
    return m_wallet->LockCoin(output);
}
void WalletImpl::unlockCoin(const COutPoint& output)
{
    LOCK(m_wallet->cs_wallet);
    return m_wallet->UnlockCoin(output);
}
bool WalletImpl::isLockedCoin(const COutPoint& output)
{
    LOCK(m_wallet->cs_wallet);
    return m_wallet->IsLockedCoin(output.hash, output.n);
}
void WalletImpl::listLockedCoins(std::vector<COutPoint>& outputs)
{
    LOCK(m_wallet->cs_wallet);
    return m_wallet->ListLockedCoins(outputs);
}
CTransactionRef WalletImpl::createTransaction(const std::vector<CRecipient>& recipients,
    const CCoinControl& coin_control,
    bool sign,
    int& change_pos,
    CAmount& fee,
    bilingual_str& fail_reason)
{
    LOCK(m_wallet->cs_wallet);
    CTransactionRef tx;
    if (!m_wallet->CreateTransaction(recipients, tx, fee, change_pos,
            fail_reason, coin_control, sign)) {
        return {};
    }
    return tx;
}
void WalletImpl::commitTransaction(CTransactionRef tx,
    WalletValueMap value_map,
    WalletOrderForm order_form)
{
    LOCK(m_wallet->cs_wallet);
    m_wallet->CommitTransaction(std::move(tx), std::move(value_map), std::move(order_form));
}
bool WalletImpl::transactionCanBeAbandoned(const uint256& txid) { return m_wallet->TransactionCanBeAbandoned(txid); }
bool WalletImpl::abandonTransaction(const uint256& txid)
{
    LOCK(m_wallet->cs_wallet);
    return m_wallet->AbandonTransaction(txid);
}
bool WalletImpl::transactionCanBeBumped(const uint256& txid)
{
    return feebumper::TransactionCanBeBumped(*m_wallet.get(), txid);
}
bool WalletImpl::createBumpTransaction(const uint256& txid,
    const CCoinControl& coin_control,
    std::vector<bilingual_str>& errors,
    CAmount& old_fee,
    CAmount& new_fee,
    CMutableTransaction& mtx)
{
    return feebumper::CreateRateBumpTransaction(*m_wallet.get(), txid, coin_control, errors, old_fee, new_fee, mtx) == feebumper::Result::OK;
}
bool WalletImpl::signBumpTransaction(CMutableTransaction& mtx) { return feebumper::SignTransaction(*m_wallet.get(), mtx); }
bool WalletImpl::commitBumpTransaction(const uint256& txid,
    CMutableTransaction&& mtx,
    std::vector<bilingual_str>& errors,
    uint256& bumped_txid)
{
    return feebumper::CommitTransaction(*m_wallet.get(), txid, std::move(mtx), errors, bumped_txid) ==
           feebumper::Result::OK;
}
CTransactionRef WalletImpl::getTx(const uint256& txid)
{
    LOCK(m_wallet->cs_wallet);
    auto mi = m_wallet->mapWallet.find(txid);
    if (mi != m_wallet->mapWallet.end()) {
        return mi->second.tx;
    }
    return {};
}
WalletTx WalletImpl::getWalletTx(const uint256& txid)
{
    LOCK(m_wallet->cs_wallet);
    auto mi = m_wallet->mapWallet.find(txid);
    if (mi != m_wallet->mapWallet.end()) {
        return MakeWalletTx(*m_wallet, mi->second);
    }
    return {};
}
std::vector<WalletTx> WalletImpl::getWalletTxs()
{
    LOCK(m_wallet->cs_wallet);
    std::vector<WalletTx> result;
    result.reserve(m_wallet->mapWallet.size());
    for (const auto& entry : m_wallet->mapWallet) {
        result.emplace_back(MakeWalletTx(*m_wallet, entry.second));
    }
    return result;
}
bool WalletImpl::tryGetTxStatus(const uint256& txid,
    interfaces::WalletTxStatus& tx_status,
    int& num_blocks,
    int64_t& block_time)
{
    TRY_LOCK(m_wallet->cs_wallet, locked_wallet);
    if (!locked_wallet) {
        return false;
    }
    auto mi = m_wallet->mapWallet.find(txid);
    if (mi == m_wallet->mapWallet.end()) {
        return false;
    }
    num_blocks = m_wallet->GetLastBlockHeight();
    block_time = -1;
    CHECK_NONFATAL(m_wallet->chain().findBlock(m_wallet->GetLastBlockHash(), FoundBlock().time(block_time)));
    tx_status = MakeWalletTxStatus(*m_wallet, mi->second);
    return true;
}
WalletTx WalletImpl::getWalletTxDetails(const uint256& txid,
    WalletTxStatus& tx_status,
    WalletOrderForm& order_form,
    bool& in_mempool,
    int& num_blocks)
{
    LOCK(m_wallet->cs_wallet);
    auto mi = m_wallet->mapWallet.find(txid);
    if (mi != m_wallet->mapWallet.end()) {
        num_blocks = m_wallet->GetLastBlockHeight();
        in_mempool = mi->second.InMempool();
        order_form = mi->second.vOrderForm;
        tx_status = MakeWalletTxStatus(*m_wallet, mi->second);
        return MakeWalletTx(*m_wallet, mi->second);
    }
    return {};
}
TransactionError WalletImpl::fillPSBT(int sighash_type,
    bool sign,
    bool bip32derivs,
    PartiallySignedTransaction& psbtx,
    bool& complete,
    size_t* n_signed)
{
    return m_wallet->FillPSBT(psbtx, complete, sighash_type, sign, bip32derivs, n_signed);
}
WalletBalances WalletImpl::getBalances()
{
    LOCK(m_wallet->cs_wallet);
    const auto bal = m_wallet->GetBalance();
    WalletBalances result;
    result.balance = bal.m_mine_trusted;
    result.unconfirmed_balance = bal.m_mine_untrusted_pending;
    result.immature_balance = bal.m_mine_immature;
    result.have_watch_only = haveWatchOnly();
    if (result.have_watch_only) {
        result.watch_only_balance = bal.m_watchonly_trusted;
        result.unconfirmed_watch_only_balance = bal.m_watchonly_untrusted_pending;
        result.immature_watch_only_balance = bal.m_watchonly_immature;
    }
    return result;
}
bool WalletImpl::tryGetBalances(WalletBalances& balances, uint256& block_hash)
{
    TRY_LOCK(m_wallet->cs_wallet, locked_wallet);
    if (!locked_wallet) {
        return false;
    }
    block_hash = m_wallet->GetLastBlockHash();
    balances = getBalances();
    return true;
}
CAmount WalletImpl::getBalance()
{
    LOCK(m_wallet->cs_wallet);
    return m_wallet->GetBalance().m_mine_trusted;
}
CAmount WalletImpl::getAvailableBalance(const CCoinControl& coin_control)
{
    return m_wallet->GetAvailableBalance(&coin_control);
}
isminetype WalletImpl::txinIsMine(const CTxIn& txin)
{
    LOCK(m_wallet->cs_wallet);
    return m_wallet->IsMine(txin);
}
isminetype WalletImpl::txoutIsMine(const CTxOut& txout)
{
    LOCK(m_wallet->cs_wallet);
    return m_wallet->IsMine(txout);
}
CAmount WalletImpl::getDebit(const CTxIn& txin, isminefilter filter)
{
    LOCK(m_wallet->cs_wallet);
    return m_wallet->GetDebit(txin, filter);
}
CAmount WalletImpl::getCredit(const CTxOut& txout, isminefilter filter)
{
    LOCK(m_wallet->cs_wallet);
    return m_wallet->GetCredit(txout, filter);
}
Wallet::CoinsList WalletImpl::listCoins()
{
    LOCK(m_wallet->cs_wallet);
    Wallet::CoinsList result;
    for (const auto& entry : m_wallet->ListCoins()) {
        auto& group = result[entry.first];
        for (const auto& coin : entry.second) {
            group.emplace_back(COutPoint(coin.tx->GetHash(), coin.i),
                MakeWalletTxOut(*m_wallet, *coin.tx, coin.i, coin.nDepth));
        }
    }
    return result;
}
std::vector<WalletTxOut> WalletImpl::getCoins(const std::vector<COutPoint>& outputs)
{
    LOCK(m_wallet->cs_wallet);
    std::vector<WalletTxOut> result;
    result.reserve(outputs.size());
    for (const auto& output : outputs) {
        result.emplace_back();
        auto it = m_wallet->mapWallet.find(output.hash);
        if (it != m_wallet->mapWallet.end()) {
            int depth = it->second.GetDepthInMainChain();
            if (depth >= 0) {
                result.back() = MakeWalletTxOut(*m_wallet, it->second, output.n, depth);
            }
        }
    }
    return result;
}
CAmount WalletImpl::getRequiredFee(unsigned int tx_bytes) { return GetRequiredFee(*m_wallet, tx_bytes); }
CAmount WalletImpl::getMinimumFee(unsigned int tx_bytes,
    const CCoinControl& coin_control,
    int* returned_target,
    FeeReason* reason)
{
    FeeCalculation fee_calc;
    CAmount result;
    result = GetMinimumFee(*m_wallet, tx_bytes, coin_control, &fee_calc);
    if (returned_target) *returned_target = fee_calc.returnedTarget;
    if (reason) *reason = fee_calc.reason;
    return result;
}
unsigned int WalletImpl::getConfirmTarget() { return m_wallet->m_confirm_target; }
bool WalletImpl::hdEnabled() { return m_wallet->IsHDEnabled(); }
bool WalletImpl::canGetAddresses() { return m_wallet->CanGetAddresses(); }
bool WalletImpl::privateKeysDisabled() { return m_wallet->IsWalletFlagSet(WALLET_FLAG_DISABLE_PRIVATE_KEYS); }
OutputType WalletImpl::getDefaultAddressType() { return m_wallet->m_default_address_type; }
CAmount WalletImpl::getDefaultMaxTxFee() { return m_wallet->m_default_max_tx_fee; }
void WalletImpl::remove()
{
    RemoveWallet(m_wallet);
}
bool WalletImpl::isLegacy() { return m_wallet->IsLegacy(); }
std::unique_ptr<Handler> WalletImpl::handleUnload(UnloadFn fn)
{
    return MakeHandler(m_wallet->NotifyUnload.connect(fn));
}
std::unique_ptr<Handler> WalletImpl::handleShowProgress(ShowProgressFn fn)
{
    return MakeHandler(m_wallet->ShowProgress.connect(fn));
}
std::unique_ptr<Handler> WalletImpl::handleStatusChanged(StatusChangedFn fn)
{
    return MakeHandler(m_wallet->NotifyStatusChanged.connect([fn](CWallet*) { fn(); }));
}
std::unique_ptr<Handler> WalletImpl::handleAddressBookChanged(AddressBookChangedFn fn)
{
    return MakeHandler(m_wallet->NotifyAddressBookChanged.connect(
        [fn](CWallet*, const CTxDestination& address, const std::string& label, bool is_mine,
            const std::string& purpose, ChangeType status) { fn(address, label, is_mine, purpose, status); }));
}
std::unique_ptr<Handler> WalletImpl::handleTransactionChanged(TransactionChangedFn fn)
{
    return MakeHandler(m_wallet->NotifyTransactionChanged.connect(
        [fn](CWallet*, const uint256& txid, ChangeType status) { fn(txid, status); }));
}
std::unique_ptr<Handler> WalletImpl::handleWatchOnlyChanged(WatchOnlyChangedFn fn)
{
    return MakeHandler(m_wallet->NotifyWatchonlyChanged.connect(fn));
}
std::unique_ptr<Handler> WalletImpl::handleCanGetAddressesChanged(CanGetAddressesChangedFn fn)
{
    return MakeHandler(m_wallet->NotifyCanGetAddressesChanged.connect(fn));
}
CWallet* WalletImpl::wallet() { return m_wallet.get(); }

std::unique_ptr<Wallet> MakeWallet(const std::shared_ptr<CWallet>& wallet) { return wallet ? MakeUnique<WalletImpl>(wallet) : nullptr; }

std::unique_ptr<ChainClient> MakeWalletClient(Chain& chain, ArgsManager& args, std::vector<std::string> wallet_filenames)
{
    return MakeUnique<WalletClientImpl>(chain, args, std::move(wallet_filenames));
}

} // namespace interfaces
