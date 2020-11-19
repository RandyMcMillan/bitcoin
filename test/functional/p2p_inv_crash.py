#!/usr/bin/env python3

from test_framework.messages import CInv, MSG_BLOCK, MSG_WITNESS_FLAG, msg_inv
from test_framework.mininode import P2PInterface
from test_framework.test_framework import BitcoinTestFramework

class InvCrash(BitcoinTestFramework):

    def set_test_params(self):
        self.setup_clean_chain = True
        self.num_nodes = 2

    def test_inv_crashing(self):

        self.log.info("Sending inv")

        raw = 0x... # your inv here
        inv = CInv(MSG_BLOCK | MSG_WITNESS_FLAG, h=raw)

        self.log.info(inv)

        self.nodes[0].p2p.send_message(msg_inv([inv]))

    def run_test(self):
        self.nodes[0].add_p2p_connection(P2PInterface())

        self.log.info("Mine a block to bail out of IBD")
        self.nodes[0].generate(nblocks=1)
        self.sync_all(self.nodes[:])
        self.test_inv_crashing()

if __name__ == '__main__':
    InvCrash().main()
