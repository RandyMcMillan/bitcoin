// Copyright (c) 2022 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <bench/bench.h>

#include <key.h>
#include <pubkey.h>
#include <random.h>
#include <span.h>

#include <cstddef>

static void BIP324_ECDH(benchmark::Bench& bench)
{
    ECC_Start();
    FastRandomContext rng;

    std::array<std::byte, 32> key_data;
    std::array<std::byte, EllSwiftPubKey::size()> our_ellswift_data;
    std::array<std::byte, EllSwiftPubKey::size()> their_ellswift_data;

    rng.fillrand(key_data);
    rng.fillrand(our_ellswift_data);
    rng.fillrand(their_ellswift_data);

    bench.batch(1).unit("ecdh").run([&] {
        CKey key;
        key.Set(UCharCast(key_data.data()), UCharCast(key_data.data()) + 32, true);
        EllSwiftPubKey our_ellswift(our_ellswift_data);
        EllSwiftPubKey their_ellswift(their_ellswift_data);

        auto ret = key.ComputeBIP324ECDHSecret(their_ellswift, our_ellswift, true);

        // Copy 8 bytes from the resulting shared secret into middle of the private key.
        std::copy(ret.begin(), ret.begin() + 8, key_data.begin() + 12);
        // Copy 12 bytes from the resulting shared secret into the middle of our ellswift key.
        std::copy(ret.begin() + 8, ret.begin() + 20, our_ellswift_data.begin() + 26);
        // Copy 12 bytes from the resulting shared secret into the middle of their ellswift key.
        std::copy(ret.begin() + 20, ret.end(), their_ellswift_data.begin() + 26);
    });

    ECC_Stop();
}

BENCHMARK(BIP324_ECDH, benchmark::PriorityLevel::HIGH);
