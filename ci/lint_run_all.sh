#!/usr/bin/env bash
#
# Copyright (c) 2019 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

export LC_ALL=C.UTF-8

set -o errexit; source ./ci/test/00_setup_env.sh
set -o errexit; source ./ci/lint/04_install.sh
set -o errexit; source ./ci/lint/06_script.sh

echo "CIRRUS_LAST_GREEN_CHANGE = $CIRRUS_LAST_GREEN_CHANGE"
echo "CIRRUS_DEFAULT_BRANCH = $CIRRUS_DEFAULT_BRANCH"
echo "COMMIT_RANGE = $COMMIT_RANGE"
