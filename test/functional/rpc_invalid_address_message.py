#!/usr/bin/env python3
# Copyright (c) 2020-2022 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test error messages for 'getaddressinfo' and 'validateaddress' RPC commands."""

from test_framework.test_framework import BitcoinTestFramework

from test_framework.util import (
    assert_equal,
    assert_raises_rpc_error,
)

BECH32_VALID = 'bcrt1qtmp74ayg7p24uslctssvjm06q5phz4yrxucgnv'
BECH32_VALID_CAPITALS = 'BCRT1QPLMTZKC2XHARPPZDLNPAQL78RSHJ68U33RAH7R'
BECH32_VALID_MULTISIG = 'bcrt1qdg3myrgvzw7ml9q0ejxhlkyxm7vl9r56yzkfgvzclrf4hkpx9yfqhpsuks'

BECH32_INVALID_BECH32 = 'bcrt1p0xlxvlhemja6c4dqv22uapctqupfhlxm9h8z3k2e72q4k9hcz7vqdmchcc'
BECH32_INVALID_BECH32M = 'bcrt1qw508d6qejxtdg4y5r3zarvary0c5xw7k35mrzd'
BECH32_INVALID_VERSION = 'bcrt130xlxvlhemja6c4dqv22uapctqupfhlxm9h8z3k2e72q4k9hcz7vqynjegk'
BECH32_INVALID_SIZE = 'bcrt1s0xlxvlhemja6c4dqv22uapctqupfhlxm9h8z3k2e72q4k9hcz7v8n0nx0muaewav25430mtr'
BECH32_INVALID_V0_SIZE = 'bcrt1qw508d6qejxtdg4y5r3zarvary0c5xw7kqqq5k3my'
BECH32_INVALID_PREFIX_WRONG_CHAIN = 'bc1pw508d6qejxtdg4y5r3zarvary0c5xw7kw508d6qejxtdg4y5r3zarvary0c5xw7k7grplx'
BECH32_INVALID_PREFIX_NO_HRP = '1mk9q5tvu03k4qnfv6j2xjycx2ceah2zw5nek7czepflkd2a2xxrqk00j84'
BECH32_TOO_SHORT = 'bc1sh0rt'
BECH32_TOO_LONG = 'bcrt1q049edschfnwystcqnsvyfpj23mpsg3jcedq9xv049edschfnwystcqnsvyfpj23mpsg3jcedq9xv049edschfnwystcqnsvyfpj23m'
BECH32_ONE_ERROR = 'bcrt1q049edschfnwystcqnsvyfpj23mpsg3jcedq9xv'
BECH32_ONE_ERROR_CAPITALS = 'BCRT1QPLMTZKC2XHARPPZDLNPAQL78RSHJ68U32RAH7R'
BECH32_TWO_ERRORS = 'bcrt1qax9suht3qv95sw33xavx8crpxduefdrsvgsklu' # should be bcrt1qax9suht3qv95sw33wavx8crpxduefdrsvgsklx
BECH32_NO_SEPARATOR = 'bcrtq049ldschfnwystcqnsvyfpj23mpsg3jcedq9xv'
BECH32_INVALID_CHAR = 'bcrt1q04oldschfnwystcqnsvyfpj23mpsg3jcedq9xv'
BECH32_MULTISIG_TWO_ERRORS = 'bcrt1qdg3myrgvzw7ml8q0ejxhlkyxn7vl9r56yzkfgvzclrf4hkpx9yfqhpsuks'
BECH32_WRONG_VERSION = 'bcrt1ptmp74ayg7p24uslctssvjm06q5phz4yrxucgnv'

BASE58_VALID = 'mipcBbFg9gMiCh81Kj8tqqdgoZub1ZJRfn'
BASE58_INVALID_PREFIX = '17VZNX1SN5NtKa8UQFxwQbFeFc3iqRYhem'
BASE58_INVALID_CHECKSUM = 'mipcBbFg9gMiCh81Kj8tqqdgoZub1ZJJfn'
BASE58_INVALID_LENGTH = '2VKf7XKMrp4bVNVmuRbyCewkP8FhGLP2E54LHDPakr9Sq5mtU2'

INVALID_ADDRESS = 'asfah14i8fajz0123f'
INVALID_ADDRESS_2 = '1q049ldschfnwystcqnsvyfpj23mpsg3jcedq9xv'
INVALID_ADDRESS_3 = '111111lllsy05pzs'

class InvalidAddressErrorMessageTest(BitcoinTestFramework):
    def add_options(self, parser):
        self.add_wallet_options(parser)

    def set_test_params(self):
        self.setup_clean_chain = True
        self.num_nodes = 1

    def check_valid(self, addr):
        info = self.nodes[0].validateaddress(addr)
        assert info['isvalid']
        assert 'error' not in info
        assert 'error_locations' not in info

    def check_invalid(self, addr, error_str, error_locations=None):
        res = self.nodes[0].validateaddress(addr)
        assert not res['isvalid']
        assert_equal(res['error'], error_str)
        if error_locations:
            assert_equal(res['error_locations'], error_locations)
        else:
            assert_equal(res['error_locations'], [])

    def test_validateaddress(self):
        # Invalid Bech32
        self.check_invalid(BECH32_INVALID_SIZE, 'Invalid Bech32 address data size')
        self.check_invalid(BECH32_INVALID_PREFIX_WRONG_CHAIN, 'Invalid chain prefix for regtest. Expected bcrt got bc')
        self.check_invalid(BECH32_INVALID_PREFIX_NO_HRP, 'Bech32(m) address decoded with error: Invalid separator position',[0])
        self.check_invalid(BECH32_INVALID_BECH32, 'Version 1+ witness address must use Bech32m checksum')
        self.check_invalid(BECH32_INVALID_BECH32M, 'Version 0 witness address must use Bech32 checksum')
        self.check_invalid(BECH32_INVALID_VERSION, 'Invalid Bech32 address witness version')
        self.check_invalid(BECH32_INVALID_V0_SIZE, 'Invalid Bech32 v0 address data size')
        self.check_invalid(BECH32_TOO_LONG, 'Bech32(m) address decoded with error: Bech32 string too long', list(range(90, 108)))
        self.check_invalid(BECH32_TOO_SHORT, 'Bech32(m) address decoded with error: Invalid separator position', [2])
        self.check_invalid(BECH32_ONE_ERROR, 'Bech32(m) address decoded with error: Invalid Bech32 checksum', [9])
        self.check_invalid(BECH32_TWO_ERRORS, 'Bech32(m) address decoded with error: Invalid Bech32 checksum', [22, 43])
        self.check_invalid(BECH32_ONE_ERROR_CAPITALS, 'Invalid address encoded as Base58 and Bech32(m) provided', [38])
        self.check_invalid(BECH32_NO_SEPARATOR, 'Bech32(m) address decoded with error: Missing separator')
        self.check_invalid(BECH32_INVALID_CHAR, 'Bech32(m) address decoded with error: Invalid Base 32 character', [8])
        self.check_invalid(BECH32_MULTISIG_TWO_ERRORS, 'Bech32(m) address decoded with error: Invalid Bech32 checksum', [19, 30])
        self.check_invalid(BECH32_WRONG_VERSION, 'Bech32(m) address decoded with error: Invalid Bech32 checksum', [5])

        # Valid Bech32
        self.check_valid(BECH32_VALID)
        self.check_valid(BECH32_VALID_CAPITALS)
        self.check_valid(BECH32_VALID_MULTISIG)

        # Invalid Base58
        self.check_invalid(BASE58_INVALID_PREFIX, 'Invalid or unsupported Base58 regtest address. Expected prefix m, n, or 2')
        self.check_invalid(BASE58_INVALID_CHECKSUM, 'Invalid address encoded as Base58 and Bech32(m) provided', [4, 6, 10, 12, 16, 25, 29, 30, 31])
        self.check_invalid(BASE58_INVALID_LENGTH, 'Invalid address encoded as Base58 and Bech32(m) provided', [3, 8, 9, 11, 15, 16, 18, 19, 21, 22, 23, 27, 39, 40, 41, 44, 46, 47])

        # Valid Base58
        self.check_valid(BASE58_VALID)

        # Invalid address format
        self.check_invalid(INVALID_ADDRESS, 'Bech32(m) address decoded with error: Invalid separator position',[14])
        self.check_invalid(INVALID_ADDRESS_2, 'Bech32(m) address decoded with error: Invalid separator position',[0])
        self.check_invalid(INVALID_ADDRESS_3, 'Invalid chain prefix for regtest. Expected bcrt got 11111')


        #node = self.nodes[0]

        # Missing arg returns the help text
        #assert_raises_rpc_error(-1, "Return information about the given bitcoin address.", node.validateaddress)
        # Explicit None is not allowed for required parameters
        #assert_raises_rpc_error(-3, "JSON value of type null is not of expected type string", node.validateaddress, None)

    def test_getaddressinfo(self):
        node = self.nodes[0]

        assert_raises_rpc_error(-5, "Invalid Bech32 address data size", node.getaddressinfo, BECH32_INVALID_SIZE)
        assert_raises_rpc_error(-5, "Invalid chain prefix for regtest. Expected bcrt got bc", node.getaddressinfo, BECH32_INVALID_PREFIX_WRONG_CHAIN)
        assert_raises_rpc_error(-5, "Invalid or unsupported Base58 regtest address. Expected prefix m, n, or 2", node.getaddressinfo, BASE58_INVALID_PREFIX)
        assert_raises_rpc_error(-5, "Bech32(m) address decoded with error: Invalid separator position", node.getaddressinfo, INVALID_ADDRESS)

    def run_test(self):
        self.test_validateaddress()

        if self.is_wallet_compiled():
            self.init_wallet(node=0)
            self.test_getaddressinfo()


if __name__ == '__main__':
    InvalidAddressErrorMessageTest().main()
