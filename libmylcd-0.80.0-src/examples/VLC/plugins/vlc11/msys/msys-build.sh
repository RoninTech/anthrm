cp /usr/win32/share/aclocal/* m4/
cp /usr/share/aclocal/* m4/
PATH=/usr/win32/bin:$PATH ./bootstrap
sh extras/package/win32/configure-msys.sh
./compile
# PATH=/usr/win32/bin:$PATH make

