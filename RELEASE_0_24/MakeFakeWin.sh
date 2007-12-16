#!/bin/bash

cat > wine_linux_launcher.c <<EOF
#include <windows.h>
#include <stdio.h>
#include <unistd.h>

#define PACKET_SZ 32
int main(int argc,char *const argv[])
{
    int found=0;
    FILE *res_stream;
    FILE *stream;
    char list[PACKET_SZ];
    char stdoutfile[1024] = "";
    char returnfile[1024] = "";
    char cmd[4096] = "";
    int  i, numread, numwritten;
    sprintf(cmd, "wine_linux_launcher.sh %s ", argv[0]);
    for(i=1; i<argc; i++)
        sprintf(cmd, "%s %s", cmd, argv[i]);
    sprintf(stdoutfile, "%s_stdout", argv[0]);
    sprintf(returnfile, "%s_return", argv[0]);
    remove(stdoutfile);
    remove(returnfile);
    system(cmd);
    while(!found){
        if( res_stream = fopen( returnfile, "r+t" ) )
        {
              found=1;
              numread=PACKET_SZ;
              stream = fopen( stdoutfile, "r+t" );
              while(numread == PACKET_SZ){
                 numread = fread( list, sizeof( char ), PACKET_SZ, stream );
                 printf( "%.*s", numread, list);
              }
              fclose(stream);
              numread = fread( list, sizeof( char ), PACKET_SZ, res_stream );
              fclose(res_stream);
              remove(stdoutfile);
              remove(returnfile);
              return atoi(list);
        } else {
            Sleep(100);
        }
    }
    return 0;
}
EOF

$1-gcc wine_linux_launcher.c -o wine_linux_launcher.exe

cat > wine_linux_launcher.sh <<EOF
#!/bin/bash
exename=\$1
bzname=\`basename \$exename .exe\`
echo  '>>> '\$exename
shift
case \$bzname in
    gcc|g++|dllwrap|windres)
        for j in "\$@"; do
            if [ "\$j" == "--output-lib" ];then
                bzname=dllwrap
            fi
            if [ "\`echo \$j | head -c2\`" = "-I" -a "\`echo \$j |head -c4 |tail -c1\`" = ":" ]; then
                badpath=\`echo \$j | sed 's/^-I//'\`
                res=\$res\ "-I\`winepath \$badpath\`"
            elif [ "\`echo \$j | head -c2\`" = "-L" -a "\`echo \$j |head -c4 |tail -c1\`" = ":" ]; then
                badpath=\`echo \$j | sed 's/^-L//'\`
                res=\$res\ "-L\`winepath \$badpath\`"
            elif [ "\`echo \$j | head -c5\`" = "-lwx_" ]; then
                res=\$res\ \${j}.dll
            else
                res=\$res\ "\`echo \$j|tr '\\' '/'\`"
            fi
        done
        echo  '>>> $1-'\$bzname \$res
        $1-\$bzname \$res > \${exename}_stdout
    ;;
    wx-config)
        /opt/wx/2.8/bin/wx-config "\$@" > \${exename}_stdout
    ;;
esac
echo \$? > \${exename}_return
EOF
chmod +x wine_linux_launcher.sh

for i in gcc g++ dllwrap windres wx-config; do
ln -sf wine_linux_launcher.exe $i.exe
done
