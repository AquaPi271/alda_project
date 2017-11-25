#!/usr/bin/perl

use English;
use strict;

my (@samples) = 
(
 "TSAMPLE_M1000_U500",   "TSAMPLE_M3000_U500",   "TSAMPLE_M5000_U500",   "TSAMPLE_M7000_U500",   "TSAMPLE_M9000_U500",
 "TSAMPLE_M1000_U1500",  "TSAMPLE_M3000_U1500",  "TSAMPLE_M5000_U1500",  "TSAMPLE_M7000_U1500",  "TSAMPLE_M9000_U1500",
 "TSAMPLE_M1000_U2500",  "TSAMPLE_M3000_U2500",  "TSAMPLE_M5000_U2500",  "TSAMPLE_M7000_U2500",  "TSAMPLE_M9000_U2500",
 "TSAMPLE_M1000_U3500",  "TSAMPLE_M3000_U3500",  "TSAMPLE_M5000_U3500",  "TSAMPLE_M7000_U3500",  "TSAMPLE_M9000_U3500",
 "TSAMPLE_M1000_U4500",  "TSAMPLE_M3000_U4500",  "TSAMPLE_M5000_U4500",  "TSAMPLE_M7000_U4500",  "TSAMPLE_M9000_U4500"
);

my ($sample);
my ($seed) = 2; 

foreach $sample (@samples) {
    printf(STDERR "==> ".$sample." <==\n");
    my ($file) = "TOP_SAMPLES/".$sample;
    my ($command) = "bin/ubcf_metric ".$file." ".$seed." 10 0.9 15 ";
    my ($command_pearson) = $command."p";
    my ($command_cosine) = $command."c";
    my ($base_command) = "bin/base_metric ".$file." ".$seed." 10 0.9";
    printf("BASE   :  ".$base_command." : ");
    system($base_command);
    printf("PEARSON:  ".$command_pearson." : ");
    system($command_pearson);
    printf("COSINE :  ".$command_cosine." : ");
    system($command_cosine);
}
# seed = 1, 0.796379  k = 32  base = 0.810257
# seed = 2, 0.922813  k = 25  base = 0.932677
# seed = 3,                   base = 0.851404
