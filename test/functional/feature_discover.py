#!/usr/bin/env python3
# Copyright (c) 2022 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
import socket
from socket import inet_pton, AF_INET, AF_INET6

from test_framework.test_framework import BitcoinTestFramework
from test_framework.util import (
    assert_equal,
    assert_greater_than,
)

def assertAddressStringTests():
    assert(inet_pton(AF_INET, '255.255.255.255'))
    assert(inet_pton(AF_INET, "255.255.255.255"))
    assert(inet_pton(AF_INET6, '::FFFF:192.1.1.1'))
    assert(inet_pton(AF_INET6, "::FFFF:192.1.1.1"))

def testAddressDict(self,local_addrs):
    if (socket.AF_UNIX):
        for address in local_addrs:
            score, score_int = address.popitem()
            port, port_int = address.popitem()
            address, address_string = address.popitem()
            self.log.info("Test if the IPv4/6 addresses are valid %s",address_string)
            socket.inet_pton(socket.AF_INET6, address_string)

class DiscoverTest(BitcoinTestFramework):
    """Test -discover command."""
    def set_test_params(self):
        self.setup_clean_chain = True
        self.num_nodes = 1
        self.extra_args = [["-discover=0"]]

    def run_test(self):
        self.log.info("Test node (-discover=0) has 0 local addresses")
        assert_equal(self.nodes[0].getnetworkinfo()["localaddresses"], [])
        assertAddressStringTests()

        # default cases
        self.log.info("Restart node with -listen -discover")
        self.log.info("and check there are local addresses")
        self.restart_node(0, ["-listen", "-discover"])
        local_addrs = list(self.nodes[0].getnetworkinfo()["localaddresses"])
        assert_greater_than(len(local_addrs), 0)
        testAddressDict(self,local_addrs)

        # additional cases
        self.log.info("Restart node with -listen=1 -discover")
        self.log.info("and check there are local addresses")
        self.restart_node(0, ["-listen=1", "-discover"])
        local_addrs = list(self.nodes[0].getnetworkinfo()["localaddresses"])
        assert_greater_than(len(local_addrs), 0)
        testAddressDict(self,local_addrs)

        self.log.info("Restart node with -listen -discover=1")
        self.log.info("and check there are local addresses")
        self.restart_node(0, ["-listen", "-discover=1"])
        local_addrs = list(self.nodes[0].getnetworkinfo()["localaddresses"])
        assert_greater_than(len(local_addrs), 0)
        testAddressDict(self,local_addrs)

        self.log.info("Restart node with -listen=1 -discover=1")
        self.log.info("and check there are local addresses")
        self.restart_node(0, ["-listen=1", "-discover=1"])
        local_addrs = list(self.nodes[0].getnetworkinfo()["localaddresses"])
        assert_greater_than(len(local_addrs), 0)
        testAddressDict(self,local_addrs)

if __name__ == '__main__':
    DiscoverTest().main()
