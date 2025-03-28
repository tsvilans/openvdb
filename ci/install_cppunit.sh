#!/usr/bin/env bash
# Copyright Contributors to the OpenVDB Project
# SPDX-License-Identifier: Apache-2.0

set -ex
CURL_VERSION="$1"

wget -O cppunit.tar.gz https://dev-www.libreoffice.org/src/cppunit-${CURL_VERSION}.tar.gz

tar -xzf cppunit.tar.gz

cd cppunit-${CURL_VERSION}

./configure

make -j8
make install
