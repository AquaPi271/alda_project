#!/usr/bin/perl

use English;
use strict;

my (@files) =
    ( "UBCF_K_SEARCH_COSINE_DENORMALIZED",
      "IBCF_K_SEARCH_COSINE_DENORMALIZED",
      "UBCF_K_SEARCH_COSINE_NORMALIZED",
      "IBCF_K_SEARCH_COSINE_NORMALIZED",
      "UBCF_K_SEARCH_PEARSON_DENORMALIZED",
      "IBCF_K_SEARCH_PEARSON_DENORMALIZED",  
      "UBCF_K_SEARCH_PEARSON_NORMALIZED",
      "IBCF_K_SEARCH_PEARSON_NORMALIZED"
    );

my ($file);

my ($data) = {};

foreach $file (@files) {
    open(FILE, "<".$file) || die("Failed to open file ".$file);

    my ($line);

    while($line = <FILE>) {
	chomp($line);
	if( $line =~ /Final RMSE = (\S+)\s+k = (\d+)/ ) {
	    $data->{$file}->{$2} = $1;
	} else {
	    printf(STDERR "Error:  Did not recognize line:\n".$line."\n");
	    exit(1);
	}
    }
    
    close(FILE);
}

my ($k);

print("k");
foreach $file (@files) {
    print(" ".$file);
}
print("\n");
for( $k = 1; $k < 18; ++$k ) {
    print($k);
    foreach $file (@files) {
	print(" ".$data->{$file}->{$k});
    }
    print("\n");
}
	    
