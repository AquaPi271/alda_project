#!/usr/bin/perl

use English;
use strict;

my (@samples) = 
(
 "TSAMPLE_M5000_U2500",  "TSAMPLE_M7000_U2500",  "TSAMPLE_M9000_U2500",
 "TSAMPLE_M1000_U3500",  "TSAMPLE_M3000_U3500",  "TSAMPLE_M5000_U3500",  "TSAMPLE_M7000_U3500",  "TSAMPLE_M9000_U3500",
 "TSAMPLE_M1000_U4500",  "TSAMPLE_M3000_U4500",  "TSAMPLE_M5000_U4500",  "TSAMPLE_M7000_U4500",  "TSAMPLE_M9000_U4500"
);

my ($sample);
my ($seed) = 1; 

# Need to run this one from missing.

my ($special) = "bin/ibcf_metric TOP_SAMPLES/TSAMPLE_M3000_U2500 ".$seed." 10 0.9 12 c 1";
printf("COSINE :  ".$special." : ");
system($special);

foreach $sample (@samples) {
    printf(STDERR "==> ".$sample." <==\n");
    my ($file) = "TOP_SAMPLES/".$sample;
    my ($command_pearson) = "bin/ibcf_metric ".$file." ".$seed." 10 0.9 12 p 1";
    my ($command_cosine) = "bin/ibcf_metric ".$file." ".$seed." 10 0.9 12 c 1";
    my ($base_command) = "bin/base_metric ".$file." ".$seed." 10 0.9";
    printf("BASE   :  ".$base_command." : ");
    system($base_command);
    printf("PEARSON:  ".$command_pearson." : ");
    system($command_pearson);
    printf("COSINE :  ".$command_cosine." : ");
    system($command_cosine);
}

