Sonim AT Commands
=================

Filesystem access::

    at*list=<path> - list directory content
                                         (0=file, 1=subdirectory)
    at*mkdir=<path> - make directory
    at*rmdir=<path> - remove directory
    at*remove=<path> - remove file
    at*move=<srcpath>,<dstpath> - ? copy (move?) files
    at*startul=<srcpath> - prepare file to upload (from phone)
                                         returned data:
                                         *STARTUL: <filesize_in_bytes>
    at*startdl=<dstpath>,<filesize> - prepare file to download (to phone)
    at*get - get base64 coded data chunk
                                         returned data:
                                         *GET: <chunklen>,<data>
    at*get - get base64 coded data chunk
                                         returned data:
                                         *GET: <chunklen>,<data>
    at*put=<no>,<len>,<data>,<chck> - put base64 coded data chunk
                                         (no is chunk number, starting from 0)
                                         (len is chunk length)
                                         (last 4 characters is checksum ?)
    at*end - end/finish file transfer operation
    -------------------
    at*syph=?,?.?,<path> - ? (add downloaded record to phonebook?)
                                         at*syph=0,1,%d,%s
                                         EXAMPLE:
    AT*SYPH=0,1,74,/app/dir/tmp.dat
    at*sysm=0,1,%d - ? SMS handling

    ---
    Phone has at least two directories from root, /app and /app3 .
    at*list=/ gives error.

