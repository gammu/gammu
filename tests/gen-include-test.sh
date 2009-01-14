#!/bin/sh

# Generates set of files which simply include one of Gammu headers
# This verifies
#  - header completeness
#  - header reneterability

tmpcmake=`mktemp`
start='# Auto generated include tests begin'
end='# Auto generated include tests end'

cecho() {
	echo "$@" >> $tmpcmake
}

cecho "$start"
cecho "# Do not modify this section, change gen-include-test.sh instead"

for x in `ls ../include/ | grep -v gammu.h` ; do
	noext=${x%.h}
	base=${noext#gammu-}
	filename=include-$base.c
	executable=include-$base
	(
		echo "/* Automatically generated test for validating header file $x */"
		echo "/* See gen-include-test.sh for details */"
		echo
		echo "#include <$x>"
		echo "#include <$x>"
		echo
		echo "/* We do not want to push another header, so we need to copy definiton of UNUSED */"
		echo "#ifndef UNUSED"
		echo "# if __GNUC__"
		echo "#  define UNUSED __attribute__ ((unused))"
		echo "# else"
		echo "#  define UNUSED"
		echo "# endif"
		echo "#endif"
		echo
		echo "int main(int argc UNUSED, char** argv UNUSED) {"
		echo "	return 0;"
		echo "}"
	) >  $filename


	cecho
	cecho "# Test for header $x"
	cecho "add_executable($executable $filename)"
	cecho "target_link_libraries($executable libGammu)"
	cecho "add_test($executable \"\${GAMMU_TEST_PATH}/$executable\${GAMMU_TEST_SUFFIX}\")"

done

cecho
cecho "$end"

umask 077
sed -e "/^$start/,/^$end/{
	/^$start/r $tmpcmake
	d
	}
	" CMakeLists.txt > CMakeLists.txt.new
cat CMakeLists.txt.new > CMakeLists.txt
rm -f $tmpcmake CMakeLists.txt.new
