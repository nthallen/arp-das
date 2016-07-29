#! /usr/bin/perl -w
use strict;

my $fname = "pkg_inst_dep.dot";
open(my $ofh, ">", $fname) ||
  die "Cannot write to $fname\n";
print "Creating $fname\n";
print $ofh
  "digraph G {\n",
  '  size="10,7.5"; ratio=fill; concentrate=true;', "\n";
my %hasdep;
my %isdep;
my @archives = </var/huarp/pkgcache/*.gz>;
for my $archive (@archives) {
  $archive =~
      m,^/var/huarp/pkgcache/([\w-]+)(-(?:\d+(?:\.\d+)*))\.tar\.gz$, ||
    die "archive '$archive' did not match pattern\n";
  my $pkg = $1;
  my $pkgver = "$1$2";
  $hasdep{$pkg} = 0 unless $hasdep{$pkg};
  $isdep{$pkg} = 0 unless $isdep{$pkg};
  open(my $hdr, "-|",
    "tar -Oxzf $archive var/huarp/pkg/$pkgver/Header") ||
    die "Unable to read from archive $archive\n";
  while (<$hdr>) {
    if ( m/^Requires: (.*)$/ ) {
      my @reqs = split(' ', $1); 
      print "$pkg: " . join( ', ', @reqs ) . "\n";
      for my $req (@reqs) {
        $isdep{$pkg} = 1;
        $hasdep{$req} = 1;
        print $ofh "  \"$pkg\" -> \"$req\";\n";
      }
    }
  }
}

for my $pkg (keys %hasdep) {
  print "$pkg has no dependents\n" unless $hasdep{$pkg};
  unless ($hasdep{$pkg} || $isdep{$pkg}) {
    print "$pkg has no edges\n";
    print $ofh "  \"$pkg\";\n";
  }
}

print $ofh "}\n";
close $ofh || warn "Error closing $fname\n";

#   find /var/huarp/pkg -name Header | while read hdr; do
#     pkg=${hdr#/var/huarp/pkg/}
#     pkg=${pkg%-*/Header}
#     req=`grep Requires: $hdr | sed -e 's/Requires: *//'`
#     echo "Package '$pkg' requires '$req'" >&2
#     for dep in $req; do
#       dep=${dep%-\[0-9\]*}
#       echo "  \"$pkg\" -> \"$dep\";"
#     done
#   done
# 
#   echo '}'
# } >pkg_inst_dep.dot
# # echo "Running dot"
# # dot -Tpdf -opkg_inst_dep.pdf pkg_inst_dep.dot
# # echo "Result in pkg_inst_dep.pdf"
