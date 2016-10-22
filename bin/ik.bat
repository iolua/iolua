@echo off

set LUA_PATH=%IOLUA_HOME%\ik\src\main\lua\?.lua

iolua %IOLUA_HOME%/ik/src/main/lua/ik.lua %*