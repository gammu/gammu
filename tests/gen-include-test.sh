#!/bin/sh

# Generates set of files which simply include one of Gammu headers
# This verifies
#  - header completeness
#  - header reneterability

echo "# Auto generated include tests begin"

for x in `ls ../include/ | grep -v gammu.h` ; do 
	noext=${x%.h}
	base=${noext#gammu-}
	filename=include-$base.c
	executable=include-$base
	( 
		echo "/* Automatically generated test for validating header file $x */"
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


	echo
	echo "# Test for header $x"
	echo "add_executable($executable $filename)"
	echo "if (CROSS_MINGW)"
    echo "    set_target_properties ($executable PROPERTIES PREFIX \"\" SUFFIX \".exe\")"
	echo "endif (CROSS_MINGW)"
	echo "target_link_libraries($executable libGammu)"
	echo "add_test($executable \"\${GAMMU_TEST_PATH}/$executable\${GAMMU_TEST_SUFFIX}\")"

done

echo
echo "# Auto generated include tests end"
