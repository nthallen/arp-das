#! /usr/bin/perl -w
use strict;

my @files = </etc/openssl/certs/*.pem>;
my @id = map { m/^.*-rootcert-(\d+)\.pem$/; [ $1, $_] } @files;
my @sid = sort { $a->[0] <=> $b->[0] } @id;
my @sfiles = map $_->[1], @sid;

my $fh;

open($fh, ">", "MANIFEST_certs") ||
  die "Unable to write to MANIFEST_certs";
print $fh map { s,^/,,; "$_\n" } @sfiles;
