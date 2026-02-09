#!/usr/bin/env python
# /// script
# requires-python = ">=3.11"
# dependencies = [
#     "unidecode==1.4.0",
#     "requests==2.32.5",
# ]
# ///
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# Copyright (c) 2015 - 2018 Michal Cihar <michal@cihar.com>
#
"""Generates gsmnet databse from wikipedia."""

import re

import requests
from unidecode import unidecode

URL = "https://en.wikipedia.org/w/index.php?title={}&action=raw"

COUNTRY = "Mobile_country_code"

PAGES = [
    "Mobile_network_codes_in_ITU_region_2xx_(Europe)",
    "Mobile_network_codes_in_ITU_region_3xx_(North_America)",
    "Mobile_network_codes_in_ITU_region_4xx_(Asia)",
    "Mobile_network_codes_in_ITU_region_5xx_(Oceania)",
    "Mobile_network_codes_in_ITU_region_6xx_(Africa)",
    "Mobile_network_codes_in_ITU_region_7xx_(South_America)",
]

TABLE_RE = re.compile(
    r"^\|[ \t]*(?P<mcc>[0-9]+)[ \t]*\|\|[ \t]*(?P<mnc>[0-9]+)?[ \t]*\|\|",
)
WIKILINK = re.compile(r"\[\[([^|\]]+\|)?(?P<text>[^\]]+)\]\]")
URLLINK = re.compile(r"\[([^ \]]+ )(?P<text>[^\]]+)\]")


def parse_line(line):
    line = line.strip("|\r\n\t ")
    parts = line.split("||")
    return {
        "mcc": parts[0].strip(),
        "mnc": parts[1].strip(),
        "brand": parts[2].strip(),
        "operator": parts[3].strip(),
    }


def print_out(result):
    for code, name in sorted(result):
        print(
            '\t{{"{0}", "{1}"}},'.format(
                code,
                unidecode(name).replace("&amp;", "&"),
            ),
        )


def print_countries(data):
    country = None
    result = []
    code = ""
    current = set()
    for line in data.splitlines():
        if not code and re.match(r"^\|\s*[0-9]{3}$", line):
            code = line[1:].strip()
            continue
        if not code and re.match(r"^\|\s*rowspan=[0-9]+\s*\|\s*[0-9]{3}$", line):
            code = line.rsplit("|", maxsplit=1)[-1].strip()
            continue
        if code:
            if (
                re.match(r"^\|(\s*rowspan=[0-9]+\s*\|)?\s*\[\[", line)
                and "authority" not in line
            ):
                country = (
                    line.split("[[", maxsplit=1)[1]
                    .split("]]", maxsplit=1)[0]
                    .split("|", maxsplit=1)[-1]
                )
                country = (
                    country.replace("List of mobile network codes in ", "")
                    .replace("List of mobile networks in ", "")
                    .removeprefix("the ")
                )
                if ", " in country:
                    country = " ".join(reversed(country.split(", ")))
                result.append((code, country))
                code = ""
            elif line == "|-":
                code = ""
            continue

        if line.startswith("==== [["):
            country = line[7:].split("]")[0].split("|")[-1]
            current = set()
            continue
        if "International operators" in line:
            country = "International operators"
            current = set()
            continue
        if not country:
            continue
        if TABLE_RE.match(line) is None:
            continue
        match = parse_line(line)
        if match["mcc"] not in current:
            current.add(match["mcc"])
            result.append((match["mcc"], country))

    print_out(result)


def print_networks(data):
    result = []
    for line in data.splitlines():
        if TABLE_RE.match(line) is None:
            continue
        match = parse_line(line)
        if not match["mnc"]:
            continue
        if match["brand"]:
            brand = match["brand"]
        elif match["operator"]:
            brand = match["operator"]
        else:
            brand = ""
        if brand == "''Unassigned''":
            brand = ""

        brand = WIKILINK.sub(r"\g<text>", brand)
        brand = URLLINK.sub(r"\g<text>", brand)
        brand = brand.replace("<sub>2</sub>", "2")

        result.append(("{} {}".format(match["mcc"], match["mnc"]), brand))

    print_out(result)


def main():
    for name in PAGES:
        response = requests.get(
            URL.format(name),
            headers={"User-Agent": "Gammu-Generate-GSM-Net/1"},
            timeout=10,
        )
        print_networks(response.text)

    response = requests.get(
        URL.format(COUNTRY),
        headers={"User-Agent": "Gammu-Generate-GSM-Net/1"},
        timeout=10,
    )
    print_networks(response.text)
    print()
    print("-" * 80)
    print()
    print_countries(response.text)


if __name__ == "__main__":
    main()
