\%x86/etc/qcc/%! bn
\%/gcc_ntox86\|default%! d
:n
\%x86/usr/lib/gcc/%! bm
\%gcc/i386-pc-nto-qnx6%! d
:m
\%/ntoarm-% d
\%/ntomips-% d
\%/ntoppc-% d
\%/ntosh-% d
\%/arm-unknown-% d
\%/mips-unknown-% d
\%/powerpc-unknown-% d
\%/sh-unknown-% d
\%/usr/bin/\(CC\|QCC\|cc\|qcc\)$% d
\%/usr/photon/% d
\%/usr/include/\(arm\|ppc\|sh\|mips\)/% d
\%/usr/help/eclipse/% d
\%/usr/photon/% d
\%/qnx6/etc/% d
\%/target/qnx6/x86/% d
\%/target/qnx6/opt/webkit/% d
\%usr/qnx641/install/% d
\%/target/qnx6/var/state/% d
s,^/,,
