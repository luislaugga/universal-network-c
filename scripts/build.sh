#
# Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
# Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
#
# universal-network-c
# build.sh
#

#!/bin/sh

echo "Building universal-network-c"

# Get the script directory
scriptdir=`dirname $0`

# Get + Print version from git
$scriptdir/version.sh
echo "Version" `tail version.m4`

# Autogen + make check
autoreconf -fvi
mkdir build/
cd build/
../configure
make check
