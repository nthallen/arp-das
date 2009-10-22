#! /usr/local/bin/perl -w

package Makefile;
use strict;

sub read_makefile {
  my ( $macros, $filename ) =  @_;
  # print "read_makefile($filename)\n";
  open( MF, "<$filename" ) || die "Unable to open $filename\n";
  while (<MF>) {
	chomp;
	if ( /^([A-Za-z_0-9]+)=(.*)$/ ) {
	  $macros->{$1} = $2;
	} elsif ( /^([A-Za-z_0-9]+)\+=(.*)$/ ) {
	  my $rep = $macros->{$1} || "";
	  $macros->{$1} = "$rep $2";
	} elsif ( /^include\s+([\S]+)$/ ) {
	  my $pos = tell MF;
	  close MF || warn "Error closing $filename for include\n";
	  read_makefile( $macros, expand_macros( $macros, $1 ) );
	  open( MF, "<$filename" ) ||
		die "Error reopening $filename\n";
	  seek( MF, $pos, 0 ) ||
		die "Error seeking in $filename after reopening\n";
	}
  }
  close(MF) || warn "Error closing $filename\n";
}

sub expand_macros {
  my ( $macros, $val ) = @_;
  while ( $val =~ m/\$\(([A-Za-z_0-9]+)\)/ ) {
	my $var = $1;
	my $varval = expand_macro( $macros, $var );
	$val =~ s/\$\($var\)/$varval/g;
  }
  return $val;
}

my %expanding;

sub expand_macro {
  my ( $macros, $macro ) = @_;
  die "Infinite loop on macro '$macro'\n" if
	defined $expanding{$macro};
  $expanding{$macro} = 1;
  my $value = $macros->{$macro} || $ENV{$macro} || "";
  $value = expand_macros( $macros, $value );
  delete $expanding{$macro};
  $macros->{$macro} = $value;
  return $value;
}

sub deglob {
  my ( $string ) = @_;
  my ( @inputs ) = split(/\s+/, $string);
  my @outputs;
  foreach my $file ( @inputs ) {
	if ( $file =~ m/[*?\[\]]/ ) {
	  push( @outputs, glob($file) );
	} else {
	  push( @outputs, $file );
	}
  }
  return @outputs;
}

1;
