# Copyright 1999-2006 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: /var/cvsroot/gentoo-x86/app-mobilephone/gammu/gammu-1.04.0-r1.ebuild,v 1.1 2006/01/07 13:07:11 mrness Exp $

inherit eutils

MY_MAJOR_VER=${PV:0:3}

DESCRIPTION="a fork of the gnokii project, a tool to handle your cellular phone"
SRC_URI="http://www.mwiacek.com/zips/gsm/${PN}/stable/${MY_MAJOR_VER/./_}x/${P}.tar.gz"
HOMEPAGE="http://www.gammu.net/projects/gammu.php"

IUSE="debug bluetooth irda mysql"
SLOT="0"
LICENSE="GPL-2"
KEYWORDS="~amd64 ~ppc ~x86"

RDEPEND="bluetooth? ( net-wireless/bluez-libs )
	mysql? ( dev-db/mysql )"
DEPEND="${RDEPEND}
	irda? ( virtual/os-headers )"

src_unpack() {
	unpack ${A}

	epatch ${FILESDIR}/${P}-devfunc-sock.patch
}

src_compile() {
	local myconf=""
	use debug && myconf="${myconf} --enable-debug"
	use bluetooth && myconf="${myconf} --with-bluedir=/usr/lib" \
		|| myconf="${myconf} --disable-bluetooth"
	use mysql || myconf="${myconf} --disable-mysql"
	use irda || myconf="${myconf} --disable-irda"
	econf \
		--prefix=/usr \
		--enable-cb \
		--enable-7110incoming \
		${myconf} || die "configure failed"

	emake || die "make failed"
}

src_install () {
	make "DESTDIR=${D}" install || die "install failed"
	mv "${D}/usr/share/doc/${PN}" "${D}/usr/share/doc/${P}"
}
