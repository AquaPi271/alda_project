#!/usr/bin/perl

use English;
use strict;

if( scalar(@ARGV) != 1 ) {
    printf("Usage:  ".$PROGRAM_NAME." <input_measurements_file>\n");
    exit(1);
}

my ($file) = $ARGV[0];

my ($line);
my (%densities);

open(DENSITY, "<DENSITIES") || die("Failed to open DENSITIES");
while( $line = <DENSITY> ) {
    chomp($line);
    if( $line =~ /(\S+)\s+(\S+)/ ) {
	$densities{$1} = $2;
    }
}
close(DENSITY);

open(FILE, "<".$file) || die("Failed to open file ".$file);

my (%base) = ();
my (%ubcf_pearson_denorm) = ();
my (%ubcf_pearson_norm) = ();
my (%ubcf_cosine_denorm) = ();
my (%ubcf_cosine_norm) = ();
my (%ibcf_pearson_denorm) = ();
my (%ibcf_pearson_norm) = ();
my (%ibcf_cosine_denorm) = ();
my (%ibcf_cosine_norm) = ();

while( $line = <FILE> ) {
    chomp($line);
    if( $line =~ /^BASE.*TOP_SAMPLES\/TSAMPLE_(M\d+_U\d+).*RMSE\s*=\s*(\S+)\s+.*/ ) {
	if( exists( $base{$1} ) ) {
	    if( $base{$1} < $2 ) {
		$base{$1} = $2;
	    }
	} else {
	    $base{$1} = $2;
	}
    } elsif( $line =~ /^PEARSON.*ubcf_metric.*TOP_SAMPLES\/TSAMPLE_(M\d+_U\d+).* p 0 .*RMSE\s*=\s*(\S+)\s+.*/ ) {
	if( exists( $ubcf_pearson_denorm{$1} ) ) {
	    printf("Error!\n");
	    exit(1);
	}
	$ubcf_pearson_denorm{$1} = $2;
    } elsif( $line =~ /^PEARSON.*ubcf_metric.*TOP_SAMPLES\/TSAMPLE_(M\d+_U\d+).* p 1 .*RMSE\s*=\s*(\S+)\s+.*/ ) {
	if( exists( $ubcf_pearson_norm{$1} ) ) {
	    printf("Error!\n");
	    exit(1);
	}
	$ubcf_pearson_norm{$1} = $2;
    } elsif( $line =~ /^COSINE.*ubcf_metric.*TOP_SAMPLES\/TSAMPLE_(M\d+_U\d+).* c 0 .*RMSE\s*=\s*(\S+)\s+.*/ ) {
	if( exists( $ubcf_cosine_denorm{$1} ) ) {
	    printf("Error!\n");
	    exit(1);
	}
	$ubcf_cosine_denorm{$1} = $2;
    } elsif( $line =~ /^COSINE.*ubcf_metric.*TOP_SAMPLES\/TSAMPLE_(M\d+_U\d+).* c 1 .*RMSE\s*=\s*(\S+)\s+.*/ ) {
	if( exists( $ubcf_cosine_norm{$1} ) ) {
	    printf("Error!\n");
	    exit(1);
	}
	$ubcf_cosine_norm{$1} = $2;
    } elsif( $line =~ /^PEARSON.*ibcf_metric.*TOP_SAMPLES\/TSAMPLE_(M\d+_U\d+).* p 0 .*RMSE\s*=\s*(\S+)\s+.*/ ) {
	if( exists( $ibcf_pearson_denorm{$1} ) ) {
	    printf("Error!\n");
	    exit(1);
	}
	$ibcf_pearson_denorm{$1} = $2;
    } elsif( $line =~ /^PEARSON.*ibcf_metric.*TOP_SAMPLES\/TSAMPLE_(M\d+_U\d+).* p 1 .*RMSE\s*=\s*(\S+)\s+.*/ ) {
	if( exists( $ibcf_pearson_norm{$1} ) ) {
	    printf("Error!\n");
	    exit(1);
	}
	$ibcf_pearson_norm{$1} = $2;
    } elsif( $line =~ /^COSINE.*ibcf_metric.*TOP_SAMPLES\/TSAMPLE_(M\d+_U\d+).* c 0 .*RMSE\s*=\s*(\S+)\s+.*/ ) {
	if( exists( $ibcf_cosine_denorm{$1} ) ) {
	    printf("Error!\n");
	    exit(1);
	}
	$ibcf_cosine_denorm{$1} = $2;
    } elsif( $line =~ /^COSINE.*ibcf_metric.*TOP_SAMPLES\/TSAMPLE_(M\d+_U\d+).* c 1 .*RMSE\s*=\s*(\S+)\s+.*/ ) {
	if( exists( $ibcf_cosine_norm{$1} ) ) {
	    printf("Error!\n");
	    exit(1);
	}
	$ibcf_cosine_norm{$1} = $2;
    } else {
	print(">>> Error:  Unrecognized line:\n".$line."\n");
	exit(1);
    }
}

# For UBCF, already know Cosine Denormalized is worst.
#
#my (%val_label) = ();
#
#my ($label);
#foreach $label (keys(%ubcf_cosine_denorm)) {
#    if( exists( $val_label{$ubcf_cosine_denorm{$label}} ) ) {
#	die("Same float for label exists!!!!");
#    }
#    $val_label{$ubcf_cosine_denorm{$label}} = $label;
#}
#
#my (@sorted_labels_ubcf) = ();
#
#my ($val);
#foreach $val (sort numerically(keys(%val_label))) {
#    push(@sorted_labels_ubcf, $val_label{$val});
#}
#
#my ($meas) = 1;
#printf("Measurement,Base Line,Cosine Denorm, Cosine Norm, Pearson Denorm, Pearson Norm\n");
#foreach $label (@sorted_labels_ubcf) {
#    print( join(',', 
#		$meas,
#		$base{$label},
#		$ubcf_cosine_denorm{$label},
#		$ubcf_cosine_norm{$label},
#		$ubcf_pearson_denorm{$label},
#		$ubcf_pearson_norm{$label})."\n" );
#    $meas++;
#}

# For IBCF, base line is worse.
#
#my (%val_label) = ();
#
#my ($label);
#foreach $label (keys(%base)) {
#    if( exists( $val_label{$base{$label}} ) ) {
#	die("Same float for label exists!!!!");
#    }
#    $val_label{$base{$label}} = $label;
#}
#
#my (@sorted_labels_ibcf) = ();
#
#my ($val);
#foreach $val (sort numerically(keys(%val_label))) {
#    push(@sorted_labels_ibcf, $val_label{$val});
#}
#
#my ($meas) = 1;
#printf("Measurement,Base Line,Cosine Denorm, Cosine Norm, Pearson Denorm, Pearson Norm\n");
#foreach $label (@sorted_labels_ibcf) {
#    print( join(',', 
#		$meas,
#		$base{$label},
#		$ibcf_cosine_denorm{$label},
#		$ibcf_cosine_norm{$label},
#		$ibcf_pearson_denorm{$label},
#		$ibcf_pearson_norm{$label})."\n" );
#    $meas++;
#}

# Density sort

my (%val_label) = ();

my ($label);
foreach $label (keys(%densities)) {
    if( exists( $val_label{$densities{$label}} ) ) {
	die("Same float for label exists!!!!");
    }
    $val_label{$densities{$label}} = $label;
}

my (@sorted_labels_densities) = ();

my ($val);
foreach $val (sort numerically(keys(%val_label))) {
    push(@sorted_labels_densities, $val_label{$val});
}

printf("Density,Base Line,IBCF Pearson Denorm, UBCF Cosine Norm\n");
foreach $label (@sorted_labels_densities) {
    print( join(',',
		$densities{$label},
		$base{$label},
		$ibcf_pearson_norm{$label},
		$ubcf_cosine_norm{$label})."\n" );
}



sub numerically {
    return( $b <=> $a );
}
#foreach $b (sort(keys(%base))) {
#    print($b." ".$base{$b}."\n");
#}

close(FILE);

#if( exists( $base_e{$1} ) ) {
#    push( @{$base_e{$1}}, $2 );
#} else {
#    $base_e{$1} = [$2];
#}
#my ($b);
#foreach $b (sort(keys(%base_e))) {
#    print($b."\n");
#    my ($m);
#    foreach $m (@{$base_e{$b}}) {
#	print("   ".$m."\n");
#    }
#}
