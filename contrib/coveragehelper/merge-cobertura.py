#!/usr/bin/env python
"""
Cobertura XML report merger

Written for merging OpenCppCoverage reports, thus not completely supporting all
Cobertura XML attributes, only line coverage which OpenCppCoverage generates.
Anything else is simply discarded in the process.

Copyright (C) 2018 Michal Cihar <michal@cihar.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
"""
import argparse
import glob
import sys

try:
    from xml.etree.ElementTree import Element, ElementTree, SubElement, iterparse
except ImportError:
    from xml.etree.ElementTree import Element, ElementTree, SubElement, iterparse

HEADER = """<?xml version="1.0"?>
<!DOCTYPE coverage SYSTEM "http://cobertura.sourceforge.net/xml/coverage-04.dtd">
"""


def log(message=""):
    """Log message to strderr"""
    sys.stderr.write(message)
    sys.stderr.write("\n")


def read_files(names):
    """Read coverage data from input files"""
    result = {}
    lines = {}
    outfile = None
    log("Reading files:")

    for filename in names:
        log(f"* {filename}")
        for event, elem in iterparse(filename, events=("start", "end")):
            if elem.tag == "class" and event == "start":
                outfile = elem.get("filename")
                lines = result.get(outfile, {})
            elif elem.tag == "line" and event == "end":
                line = elem.get("number")
                lines[line] = lines.get(line, 0) + int(elem.get("hits"))
            elif elem.tag == "class" and event == "end":
                result[outfile] = lines
    log()

    return result


def get_line_rates(data):
    """Calculate line hit rates from raw coverage data"""
    result = {}
    total_lines = 0
    total_hits = 0
    log("Counting line rates:")
    for item in data:
        lines = len(data[item])
        hits = sum(1 for x in data[item].values() if x)
        result[item] = 1.0 * hits / lines
        log(f" * {item} = {result[item]} ({hits} / {lines})")
        total_lines += lines
        total_hits += hits

    result["_"] = 1.0 * total_hits / total_lines
    result["_hits"] = total_hits
    result["_lines"] = total_lines

    return result


def write_data(data, handle):
    """Write Cobertura XML for coverage data"""
    line_rates = get_line_rates(data)

    log("Generating output...")
    root = Element("coverage")
    root.set("line-rate", str(line_rates["_"]))
    root.set("branch-rate", "0")
    root.set("complexity", "0")
    root.set("branches-covered", "0")
    root.set("branches-valid", "0")
    root.set("timestamp", "0")
    root.set("lines-covered", str(line_rates["_hits"]))
    root.set("lines-valid", str(line_rates["_lines"]))
    root.set("version", "0")

    sources = SubElement(root, "sources")
    source = SubElement(sources, "source")
    source.text = "c:"

    packages = SubElement(root, "packages")
    package = SubElement(packages, "package")
    package.set("name", "gammu.exe")
    package.set("line-rate", str(line_rates["_"]))
    package.set("branch-rate", "0")
    package.set("complexity", "0")
    classes = SubElement(package, "classes")

    for item in data:
        obj = SubElement(classes, "class")
        obj.set("name", item.rsplit("\\", 1)[1])
        obj.set("filename", item)
        obj.set("line-rate", str(line_rates[item]))
        obj.set("branch-rate", "0")
        obj.set("complexity", "0")
        SubElement(obj, "methods")
        lines = SubElement(obj, "lines")
        for line in sorted(data[item], key=lambda x: int(x)):
            obj = SubElement(lines, "line")
            obj.set("number", line)
            obj.set("hits", str(data[item][line]))

    tree = ElementTree(root)

    handle.write(HEADER)

    tree.write(handle, xml_declaration=False)


def main():
    """Command line interface"""
    parser = argparse.ArgumentParser(
        description=sys.modules[__name__].__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument("-m", "--match", help="wildcard to match files to process")
    parser.add_argument("-o", "--output", help="output file, stdout used if omitted")
    parser.add_argument("file", nargs="*", help="files to process")
    args = parser.parse_args()
    if args.match:
        files = glob.glob(args.match)
    else:
        files = args.file

    result = read_files(files)

    if args.output:
        with open(args.output, "w") as handle:
            write_data(result, handle)
    else:
        write_data(result, sys.stdout)


if __name__ == "__main__":
    main()
