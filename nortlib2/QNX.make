#! /bin/sh

homedir=$PWD
pkg=`basename $homedir`

mkdir BLD
cd BLD
LD=wlink ../configure --disable-shared

[ -n "$MODELS" ] || MODELS="3r l s"

MODEL_ARGS_3r="-ms -3"
MODEL_ARGS_l="-ml -2"
MODEL_ARGS_s="-ms -2"

DESTDIR=$PWD/LIBS
LIBDIR=$DESTDIR/usr/local/lib

install_target=install
cd src
for MODEL in $MODELS; do
  eval "MODELARGS=\"\$MODEL_ARGS_$MODEL\""
  echo "MODEL is '$MODEL'"
  echo "MODELARGS are '$MODELARGS'"
  make $install_target DESTDIR=$DESTDIR CFLAGS="$MODELARGS" &&
  make clean
  install_target=install-libLTLIBRARIES
  for lib in $LIBDIR/lib*.a; do
	oldlib=`basename $lib`
    newlib=${oldlib#lib}
	newlib=${newlib%.a}
	newlib="$newlib$MODEL.lib"
	echo mv $lib $LIBDIR/$newlib
	mv $lib $LIBDIR/$newlib
	rm $LIBDIR/${oldlib%.a}.la
  done
done
cd $homedir
pax -wv -s,^$DESTDIR,, $DESTDIR | gzip >$pkg-bin.tgz
echo "made" >../$pkg.made
