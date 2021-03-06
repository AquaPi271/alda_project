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
my ($seed) = 1; 

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

