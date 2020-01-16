#!/bin/bash
find main/ -maxdepth 1 -type f | egrep '\.(cpp|hpp|c|h|cc|hh)$' | xargs clang-format -i -style=file
