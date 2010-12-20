\%x86/etc/qcc/%! bn
\%/gcc_ntox86\|default%! d
:n
\%x86/usr/lib/gcc/%! bm
\%gcc/i[34]86-pc-nto-qnx6%! d
:m
\%/ntoarm-% d
\%/ntoarmv7-% d
\%/ntomips-% d
\%/ntoppc-% d
\%/ntosh-% d
\%/arm-unknown-% d
\%/mips-unknown-% d
\%/powerpc-unknown-% d
\%/sh-unknown-% d
\%/usr/bin/\(CC\|QCC\|cc\|qcc\)$% d
\%/usr/photon% d
\%/usr/include/\(arm\|ppc\|sh\|mips\)/% d
\%/usr/help/eclipse/% d
\%/qnx6/usr/photon% d
\%/target/qnx6/x86/usr/bin/% d
\%/target/qnx6/x86/usr/libexec/% d
\%/target/qnx6/x86/usr/photon/bin/% d
\%/target/qnx6/x86/usr/photon/dll/% d
\%/target/qnx6/x86/usr/photon/savers/% d
\%/target/qnx6/x86/usr/sbin/% d
\%/qnx6/etc/% d
\%/target/qnx6/x86/boot/% d
\%/target/qnx6/x86/bin/% d
\%/target/qnx6/x86/sbin/% d
\%/target/qnx6/opt/webkit/% d
\%usr/qnx6[0-9]*/install% d
\%/target/qnx6/var/state/% d
s,^/,,
