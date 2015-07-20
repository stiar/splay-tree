#!/usr/bin/env python

import sys
import random

if __name__ == '__main__':
    for x in xrange(1000000):
        action = random.choice(['check', 'insert', 'count'])
        if action != 'count':
            n = random.randint(0, 1000)
            sys.stdout.write(action + ' ' + str(n) + '\n')
        else:
            sys.stdout.write(action + '\n')

