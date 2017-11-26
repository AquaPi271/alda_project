#!/usr/bin/perl

use English;
use strict;

my ($k);
for( $k = 1; $k < 18; ++$k ) {
    system("bin/ibcf_metric TOP_SAMPLES/TSAMPLE_M1000_U1500 1 10 0.8 ".$k." p");
}
