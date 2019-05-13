#!/usr/bin/env python3
# -*- mode: python -*-

import sys
import re

COMMENT = re.compile("#") 
INCLUDE = re.compile("#@include (?P<filename>\S*)")

def preprocess(f):
    for line in f:
        match = INCLUDE.match(line)
        if match:
            with open(match.group("filename")) as f:
                for line in preprocess(f):
                    yield line
        elif not COMMENT.match(line):
            yield(line)


if __name__ == "__main__":
    for line in preprocess(sys.stdin):
        print(line, end="")
