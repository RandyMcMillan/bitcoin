// Copyright (c) 2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <interfaces/txmempool.h>

#include <primitives/transaction.h>
#include <sync.h>
#include <txmempool.h>
#include <util/check.h>
#include <util/memory.h>

#include <memory>
#include <vector>

namespace interfaces {

TxMempoolImpl::TxMempoolImpl(CTxMemPool* txmempool)
{
    Assert(txmempool);
    m_pool = txmempool;
}

void TxMempoolImpl::TransactionUpdated()
{
    pool()->AddTransactionsUpdated(1);
}

void TxMempoolImpl::Check(const CCoinsViewCache& coins)
{
    pool()->check(&coins);
}

void TxMempoolImpl::RemoveRecursive(const CTransaction& tx, MemPoolRemovalReason reason)
{
    AssertLockHeld(mutex());
    pool()->removeRecursive(tx, reason);
}

void TxMempoolImpl::RemoveForReorg(const CCoinsViewCache* coins, unsigned int txmempool_height, int flags)
{
    AssertLockHeld(mutex());
    pool()->removeForReorg(coins, txmempool_height, flags);
}

void TxMempoolImpl::RemoveForBlock(const std::vector<CTransactionRef>& vtx, unsigned int block_height)
{
    AssertLockHeld(mutex());
    pool()->removeForBlock(vtx, block_height);
}

void TxMempoolImpl::UpdateTransactionsFromBlock(const std::vector<uint256>& hashes_to_update)
{
    AssertLockHeld(mutex());
    pool()->UpdateTransactionsFromBlock(hashes_to_update);
}

namespace {

class NoTxMempool final : public TxMempool
{
public:
    explicit NoTxMempool() {}

    void TransactionUpdated() override { invalidCall(); }
    void Check(const CCoinsViewCache& coins) override { invalidCall(); }
    void RemoveRecursive(const CTransaction& tx, MemPoolRemovalReason reason) override { invalidCall(); }
    void RemoveForReorg(const CCoinsViewCache* coins, unsigned int txmempool_height, int flags) override { invalidCall(); }
    void RemoveForBlock(const std::vector<CTransactionRef>& vtx, unsigned int block_height) override { invalidCall(); }
    void UpdateTransactionsFromBlock(const std::vector<uint256>& hashes_to_update) override { invalidCall(); }
    CTxMemPool* pool() override { return nullptr; }
    RecursiveMutex* mutex() override { return nullptr; }

private:
    void invalidCall() { Assert(false); }
};

} // namespace

std::unique_ptr<TxMempool> MakeTxMempool(CTxMemPool* txmempool)
{
    if (txmempool) {
        return MakeUnique<TxMempoolImpl>(txmempool);
    } else {
        return MakeUnique<NoTxMempool>();
    }
}

} // namespace interfaces
