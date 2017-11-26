#!/usr/bin/perl

use English;
use strict;

my ($k);
for( $k = 1; $k < 18; ++$k ) {
    system("bin/ubcf_metric TOP_SAMPLES/TSAMPLE_M1000_U500 2 10 0.8 ".$k." c 0");
}
