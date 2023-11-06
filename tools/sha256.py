#!/bin/python3
""" Tool to print sha256 of file """

import hashlib
import argparse

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Python hash calculator')
    parser.add_argument('-f', '--file', dest='file')
    args = parser.parse_args()
    with open(args.file, 'rb') as file:
        print(hashlib.sha256(file.read()).hexdigest())
