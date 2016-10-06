@echo off

set LUA_PATH=%IK_HOME%\src\main\lua\?.lua

iolua %IK_HOME%/src/main/lua/ik.lua %*