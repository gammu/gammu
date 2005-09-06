# Copyright 1999-2002 Gentoo Technologies, Inc.
# Distributed under the terms of the GNU General Public License, v2 or later
# $Header$

inherit eutils

PV1=${PV/./}
S=$WORKDIR/${PN}-${PV1}
DESCRIPTION="A client that plugs into your handphone"
SRC_URI="http://www.mwiacek.com/zips/gsm/gammu/older/${PN}-${PV1}.tar.gz"
HOMEPAGE="http://marcin-wiacek.fkn.pl/english/gsm/${PN}/${PN}.html"

DEPEND="virtual/x11"

SLOT="0"
LICENSE="GPL-2"
KEYWORDS="x86"

src_compile() {
    epatch ${FILESDIR}/gammu-08x-currConfig.patch
    econf --enable-cb --enable-7110incoming --enable-newcalendar
    make || die "make failed"
}

src_install() {
    make DESTDIR=${D} \
         install || die "installation failed"
    doman docs/docs/english/gammu.1
    mv ${D}/usr/share/doc/${PN} ${D}/usr/share/doc/${P}
}
