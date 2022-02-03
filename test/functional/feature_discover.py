#!/usr/bin/env python3
# Copyright (c) 2022 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test -discover command."""
import socket

from test_framework.test_framework import BitcoinTestFramework
from test_framework.util import (
    assert_equal,
    assert_greater_than,
)


class DiscoverTest(BitcoinTestFramework):
    def set_test_params(self):
        self.setup_clean_chain = True
        self.num_nodes = 1
        self.extra_args = [["-discover=0"]]

    def run_test(self):
        self.log.info("Test node (-discover=0) has 0 local addresses")
        assert_equal(self.nodes[0].getnetworkinfo()["localaddresses"], [])

        self.log.info("Restart node with -discover=1 and check there are local addresses")
        self.restart_node(0, ["-listen", "-discover=1"])
        local_addresses = list(self.nodes[0].getnetworkinfo()["localaddresses"])
        assert_greater_than(len(local_addresses), 0)

        self.log.info("Test if the addresses are valid")
        for address in local_addresses:
            socket.inet_pton(socket.AF_INET6, address["address"])


if __name__ == '__main__':
    DiscoverTest().main()
