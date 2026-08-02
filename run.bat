#!/bin/bash
@echo off
echo Building CScript2...

clear
cd build
cmake --build . --config Release
echo Build completed.
echo Running CScript...
./Cscript
cd ..