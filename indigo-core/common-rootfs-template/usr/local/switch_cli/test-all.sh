#!/bin/sh
#
# Usage:   test-all.sh [dbg-level]
# Invoke various tests cases for the Indigo CLI
#
# If $1 is specified, pass that argument as the debug argument to CLI
#

dbg_arg=""
if test -n "$1"; then dbg_arg="-d $1"; echo "Debug $dbg_arg"; fi

source /etc/find-env

# More complicated sets of tests, and test -f batching
lua cli.lua $dbg_arg -f test-port

# Test a few simple commands, and test -e batching
lua cli.lua $dbg_arg -e "cli_stat"
lua cli.lua $dbg_arg -e "history"
lua cli.lua $dbg_arg -e "help"
lua cli.lua $dbg_arg -e "help lua"
lua cli.lua $dbg_arg -e "help all"
lua cli.lua $dbg_arg -e "help port"
lua cli.lua $dbg_arg -e "help history"
lua cli.lua $dbg_arg -e "cxn_history"
lua cli.lua $dbg_arg -e "version"
lua cli.lua $dbg_arg -e "debug verbose"
lua cli.lua $dbg_arg -e "flowtable"
lua cli.lua $dbg_arg -e "flowtable brief"
lua cli.lua $dbg_arg -e "flowtable extended"
lua cli.lua $dbg_arg -e "flowtable full"
lua cli.lua $dbg_arg -e "mgmtstats"
lua cli.lua $dbg_arg -e "enable"
lua cli.lua $dbg_arg -e "echo"
lua cli.lua $dbg_arg -e "echo hello"
lua cli.lua $dbg_arg -e "echo hello 123"
lua cli.lua $dbg_arg -e "shell echo hello"
lua cli.lua $dbg_arg -e "lua x=1"
lua cli.lua $dbg_arg -e "lua print(Global.port_count)"
lua cli.lua $dbg_arg -e "lua"
lua cli.lua $dbg_arg -e "drvcmd"
lua cli.lua $dbg_arg -e "drvcmd port 10"

cp $config_dir/sysenv /tmp/sysenv.save
lua cli.lua $dbg_arg -f test-config
cp /tmp/sysenv.save $config_dir/sysenv
