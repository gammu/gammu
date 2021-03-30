#!/usr/bin/env python

import sys


def main():
    with open(sys.argv[1]) as handle:
        output = None
        for line in handle.readlines():
            if line.startswith("# path="):
                name = line.split("=", 1)[1].strip()
                print(f"Creating {name}")
                if output:
                    output.close()
                output = open(name, "w")
            elif line.startswith("<<<<<< EOF"):
                output.close()
            elif output:
                output.write(line)
        if output:
            output.close()


if __name__ == "__main__":
    main()
