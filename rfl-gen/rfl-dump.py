#!/usr/bin/env python

import sys
import rfl.proto

def Main():
    if len(sys.argv) < 2:
        print 'usage: ', sys.argv, '<rfl file>'
        return 1
    pb = open(sys.argv[1], 'rb').read()
    pkg = rfl.proto.Package()
    pkg.ParseFromString(pb[4:])

    print pkg
    return 0


if __name__ == '__main__':
    sys.exit(Main())
