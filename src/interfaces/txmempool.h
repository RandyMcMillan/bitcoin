// Copyright (c) 2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_INTERFACES_TXMEMPOOL_H
#define BITCOIN_INTERFACES_TXMEMPOOL_H

#include <primitives/transaction.h> // For CTransactionRef
#include <sync.h>                   // For RecursiveMutex
#include <txmempool.h>

#include <memory>
#include <vector>

class CCoinsViewCache;

extern RecursiveMutex cs_main;

namespace interfaces {

//! Interface for accessing a txmempool.
class TxMempool
{
public:
    virtual ~TxMempool() {}

    //! A transaction was updated
    virtual void TransactionUpdated() = 0;

    //! Run consistency check
    virtual void Check(const CCoinsViewCache& coins) = 0;

    virtual void RemoveRecursive(const CTransaction& tx, MemPoolRemovalReason reason)
        EXCLUSIVE_LOCKS_REQUIRED(mutex()) = 0;

    virtual void RemoveForReorg(const CCoinsViewCache* coins, unsigned int txmempool_height, int flags)
        EXCLUSIVE_LOCKS_REQUIRED(::cs_main, mutex()) = 0;

    //! Remove txs after block has been connected
    virtual void RemoveForBlock(const std::vector<CTransactionRef>& vtx, unsigned int block_height)
        EXCLUSIVE_LOCKS_REQUIRED(mutex()) = 0;

    virtual void UpdateTransactionsFromBlock(const std::vector<uint256>& hashes_to_update)
        EXCLUSIVE_LOCKS_REQUIRED(::cs_main, mutex()) = 0;

    //! Return pointer to internal txmempool class
    virtual CTxMemPool* pool() = 0;

    virtual RecursiveMutex* mutex() LOCK_RETURNED(pool()->cs) = 0;
};

class TxMempoolImpl final : public TxMempool
{
public:
    explicit TxMempoolImpl(CTxMemPool* txmempool);

    void TransactionUpdated() override;

    void Check(const CCoinsViewCache& coins) override;

    void RemoveRecursive(const CTransaction& tx, MemPoolRemovalReason reason) override
        EXCLUSIVE_LOCKS_REQUIRED(mutex());

    void RemoveForReorg(const CCoinsViewCache* coins, unsigned int txmempool_height, int flags) override
        EXCLUSIVE_LOCKS_REQUIRED(::cs_main, mutex());

    void RemoveForBlock(const std::vector<CTransactionRef>& vtx, unsigned int block_height) override
        EXCLUSIVE_LOCKS_REQUIRED(mutex());

    void UpdateTransactionsFromBlock(const std::vector<uint256>& hashes_to_update) override
        EXCLUSIVE_LOCKS_REQUIRED(::cs_main, mutex());

    CTxMemPool* pool() override { return m_pool; }

    RecursiveMutex* mutex() override LOCK_RETURNED(pool()->cs) { return &pool()->cs; }

private:
    CTxMemPool* m_pool = nullptr;
};

//! Return implementation of txmempool interface.
std::unique_ptr<TxMempool> MakeTxMempool(CTxMemPool* txmempool);

} // namespace interfaces

#endif // BITCOIN_INTERFACES_TXMEMPOOL_H
