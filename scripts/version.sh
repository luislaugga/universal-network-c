#
# Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
# Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
#
# universal-network-c
# version.sh
#

#!/bin/sh

scriptdir="$( cd -P "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if set `git describe --tags --long`
then
	echo "(version.sh) last commit version $1"
	echo "m4_define([VERSION_NUMBER], $1)" \
	> $scriptdir/../version.m4	
fi