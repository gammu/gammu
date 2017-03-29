#!/usr/bin/env python
from __future__ import print_function
import argparse
import glob
import sys
try:
    from xml.etree.cElementTree import iterparse, Element, SubElement, ElementTree
except ImportError:
    from xml.etree.ElementTree import iterparse, Element, SubElement, ElementTree


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
        log(' * {0} = {1}'.format(item, result[item]))
        total_lines += lines
        total_hits += hits

    result['_'] = 1.0 * total_hits / total_lines

    return result


def write_data(data, handle):
    line_rates = get_line_rates(data)

    root = Element('coverage')
    root.set('line-rate', str(line_rates['_']))
    packages = SubElement(root, 'packages')
    package = SubElement(packages, 'package')
    package.set('name', 'gammu.exe')
    package.set('line-rate', str(line_rates['_']))
    classes = SubElement(package, 'classes')

    for item in data:
        obj = SubElement(classes, 'class')
        obj.set('name', item.rsplit('\\', 1)[1])
        obj.set('filename', item)
        obj.set('line-rate', str(line_rates[item]))
        lines = SubElement(obj, 'lines')
        for line in sorted(data[item], key=lambda x: int(x)):
            obj = SubElement(lines, 'line')
            obj.set('number', line)
            obj.set('hits', str(data[item][line]))

    tree = ElementTree(root)

    tree.write(handle, xml_declaration=True)


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
