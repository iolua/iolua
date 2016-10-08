#!/bin/sh

export LUA_PATH=$IK_HOME/src/main/lua/?.lua

iolua $IK_HOME/src/main/lua/ik.lua $*