#!/usr/bin/env python
from __future__ import print_function
import argparse
import glob
import sys
try:
    from xml.etree.cElementTree import (
        iterparse, Element, SubElement, ElementTree,
    )
except ImportError:
    from xml.etree.ElementTree import (
        iterparse, Element, SubElement, ElementTree,
    )

HEADER = '''<?xml version="1.0"?>
<!DOCTYPE coverage SYSTEM "http://cobertura.sourceforge.net/xml/coverage-04.dtd">
'''


def log(message=''):
    sys.stderr.write(message)
    sys.stderr.write('\n')


def read_files(names):
    result = {}
    lines = {}
    outfile = None
    log('Reading files:')

    for filename in names:
        log('* {}'.format(filename))
        for event, elem in iterparse(filename, events=('start', 'end')):
            if elem.tag == 'class' and event == 'start':
                outfile = elem.get('filename')
                lines = result.get(outfile, {})
            elif elem.tag == 'line' and event == 'end':
                line = elem.get('number')
                lines[line] = lines.get(line, 0) + int(elem.get('hits'))
            elif elem.tag == 'class' and event == 'end':
                result[outfile] = lines
    log()

    return result


def get_line_rates(data):
    result = {}
    total_lines = 0
    total_hits = 0
    log('Counting line rates:')
    for item in data:
        lines = len(data[item])
        hits = sum([1 for x in data[item].values() if x])
        result[item] = 1.0 * hits / lines
        log(' * {0} = {1} ({2} / {3})'.format(item, result[item], hits, lines))
        total_lines += lines
        total_hits += hits

    result['_'] = 1.0 * total_hits / total_lines
    result['_hits'] = total_hits
    result['_lines'] = total_lines

    return result


def write_data(data, handle):
    line_rates = get_line_rates(data)

    log('Generating output...')
    root = Element('coverage')
    root.set('line-rate', str(line_rates['_']))
    root.set('branch-rate', '0')
    root.set('complexity', '0')
    root.set('branches-covered', '0')
    root.set('branches-valid', '0')
    root.set('timestamp', '0')
    root.set('lines-covered', str(line_rates['_hits']))
    root.set('lines-valid', str(line_rates['_lines']))
    root.set('version', '0')

    sources = SubElement(root, 'sources')
    source = SubElement(sources, 'source')
    source.text = 'c:'

    packages = SubElement(root, 'packages')
    package = SubElement(packages, 'package')
    package.set('name', 'gammu.exe')
    package.set('line-rate', str(line_rates['_']))
    package.set('branch-rate', '0')
    package.set('complexity', '0')
    classes = SubElement(package, 'classes')

    for item in data:
        obj = SubElement(classes, 'class')
        obj.set('name', item.rsplit('\\', 1)[1])
        obj.set('filename', item)
        obj.set('line-rate', str(line_rates[item]))
        obj.set('branch-rate', '0')
        obj.set('complexity', '0')
        SubElement(obj, 'methods')
        lines = SubElement(obj, 'lines')
        for line in sorted(data[item], key=lambda x: int(x)):
            obj = SubElement(lines, 'line')
            obj.set('number', line)
            obj.set('hits', str(data[item][line]))

    tree = ElementTree(root)

    handle.write(HEADER)

    tree.write(handle, xml_declaration=False)


def main():
    parser = argparse.ArgumentParser(description='Process some integers.')
    parser.add_argument(
        '-m', '--match',
        help='wildcard to match'
    )
    parser.add_argument(
        '-o', '--output',
        help='output file'
    )
    parser.add_argument(
        'files',
        nargs='*',
        help='files'
    )
    args = parser.parse_args()
    if args.match:
        files = glob.glob(args.match)
    else:
        files = args.files

    result = read_files(files)

    if args.output:
        with open(args.output, 'w') as handle:
            write_data(result, handle)
    else:
        write_data(result, sys.stdout)


if __name__ == '__main__':
    main()
