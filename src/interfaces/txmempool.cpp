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
namespace {

class TxMempoolImpl final : public TxMempool
{
public:
    explicit TxMempoolImpl(CTxMemPool* txmempool)
    {
        Assert(txmempool);
        m_txmempool = txmempool;
    }

    void TransactionUpdated() override
    {
        m_txmempool->AddTransactionsUpdated(1);
    }

    void Check(const CCoinsViewCache& coins) override
    {
        m_txmempool->check(&coins);
    }

    void RemoveRecursive(const CTransaction& tx, MemPoolRemovalReason reason) override
        EXCLUSIVE_LOCKS_REQUIRED(mutex())
    {
        AssertLockHeld(m_txmempool->cs);
        m_txmempool->removeRecursive(tx, reason);
    }

    void RemoveForReorg(const CCoinsViewCache* coins, unsigned int txmempool_height, int flags) override
        EXCLUSIVE_LOCKS_REQUIRED(::cs_main, mutex())
    {
        AssertLockHeld(m_txmempool->cs);
        m_txmempool->removeForReorg(coins, txmempool_height, flags);
    }

    void RemoveForBlock(const std::vector<CTransactionRef>& vtx, unsigned int block_height) override
        EXCLUSIVE_LOCKS_REQUIRED(mutex())
    {
        AssertLockHeld(m_txmempool->cs);
        m_txmempool->removeForBlock(vtx, block_height);
    }

    void UpdateTransactionsFromBlock(const std::vector<uint256>& hashes_to_update) override
        EXCLUSIVE_LOCKS_REQUIRED(::cs_main, mutex())
    {
        AssertLockHeld(m_txmempool->cs);
        m_txmempool->UpdateTransactionsFromBlock(hashes_to_update);
    }

    CTxMemPool* pool() override
    {
        return m_txmempool;
    }

    RecursiveMutex* mutex() override
        LOCK_RETURNED(m_txmempool->cs)
    {
        return &m_txmempool->cs;
    }
};

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
