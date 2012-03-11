#!/bin/bash

# find and run all test executables
find . -regex '^\..*test_\w*' -type f | bash
