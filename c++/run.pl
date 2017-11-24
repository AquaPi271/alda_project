#!/usr/bin/perl

my ($k);

for( $k = 1; $k < 40; ++$k ) {
    system( "bin/ubcf_metric.exe TOP_SAMPLES/TSAMPLE_M1000_U500 1 0.9 ".$k." 0.99" );
}
# seed = 1, 0.796379  k = 32  base = 0.810257
# seed = 2, 0.922813  k = 25  base = 0.932677
# seed = 3,                   base = 0.851404
