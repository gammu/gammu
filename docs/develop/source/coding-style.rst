Coding Style
============

1. use indetation, tab is tab and is 8 chars wide

2. all enums start from 1, not from 0

3. when prepare frame for phone with bitmap (and width/height info):
   Please notice, that bitmap before coding according to phone needs
   can have any dimensions. Use PHONE_GetBitmapWidthHeight to get dimensions
   of target image format, don't use width/height from bitmap structure.
