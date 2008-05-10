#!/bin/bash
exename=$1
bzname=`basename $exename .exe`
echo  '>>> '$exename
shift
case $bzname in
    gcc|g++|dllwrap|windres)
        for j in "$@"; do
            if [ "$j" == "--output-lib" ];then
                bzname=dllwrap
            fi
            if [ "`echo $j | head -c2`" = "-I" -a "`echo $j |head -c4 |tail -c1`" = ":" ]; then
                badpath=`echo $j | sed 's/^-I//'`
                res="$res \"-I`winepath "$badpath"`\""
            elif [ "`echo $j | head -c2`" = "-L" -a "`echo $j |head -c4 |tail -c1`" = ":" ]; then
                badpath=`echo $j | sed 's/^-L//'`
                res="$res \"-L`winepath "$badpath"`\""
            elif [ "`echo $j | head -c5`" = "-lwx_" ]; then
                res="$res \"${j}.dll\""
            else
                res="$res \"`echo $j|tr '\\\\' '/'`\""
            fi
        done
        echo  '>>> i586-mingw32msvc-'$bzname $res
        eval i586-mingw32msvc-$bzname $res > ${exename}_stdout
    ;;
    wx-config)
        /opt/wx/2.8/bin/wx-config "$@" > ${exename}_stdout
    ;;
esac
echo $? > ${exename}_return
