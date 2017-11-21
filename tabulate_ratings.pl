#!/usr/bin/perl

use English;
use strict;

my ($ratings_dir) = "./training_set";

my (@files) = ();
opendir( RATINGS, $ratings_dir ) || die( "Failed to open directory:  ".$ratings_dir);
@files = grep(/.*\.txt$/, readdir( RATINGS ) );
closedir( RATINGS );

my ($file);
my (%ratings) = ();
my ($mean);
my ($var);
my ($m);

$ratings{1} = 0;
$ratings{2} = 0;
$ratings{3} = 0;
$ratings{4} = 0;
$ratings{5} = 0;

my ($line);
my ($count) = 0;
my ($user) = {};
my ($movie);
my ($movie_ratings) = {};

foreach $file ( @files ) {
    open( FILE, "<".$ratings_dir."/".$file ) || die( "Failed to open file '".$file."'");
    $line = <FILE>; # skip first line for movie index
    if( $line =~ /^\s*(\d+)\s*\:\s*$/ ) {
        $movie = $1;
        $movie_ratings->{$movie} = [];
    } else {
        printf("No movie number given in file '".$file."'");
        exit( 1 );
    }
    while( $line = <FILE> ) {
        chomp($line);
	#my (@data) = ();
	#@data = split(/,/,$line);
        if( $line =~ /^\s*(\S+)\s*\,\s*(\d)\s*\,\s*(\S+)\s*$/ ) {
	    my ($val) = 1 * $data[1];
            $ratings{$val}++;
            #$user->{"id"} = $1;
            #$user->{"rating"} = $2;
            #$user->{"date"} = $3;
            #$user->{"movie"} = $movie;
            push(@{$movie_ratings->{$movie}}, $val);
        }
    }
    close( FILE );
    ++$count;
#    $m = $movie;
#    $mean = compute_mean($movie_ratings->{$m});
#    $var  = compute_variance($movie_ratings->{$m}, $mean);
    if( $count % 100 == 0 ) {
	print($count."\n");
#	printf("%d:  mean = %g variance = %g rating_count = %d\n", $m, $mean, $var, scalar(@{$movie_ratings->{$m}}));
    }
#       printf("%d => %d %d %d %d %d\n",
#              $count,
#              $ratings{1},
#              $ratings{2},
#              $ratings{3},
#              $ratings{4},
#              $ratings{5}
#           );
#    }
}

# 5561

# Low number of ratings is probably a specialized piece.
# Total number of ratings is popularity measure.

printf("%d %d %d %d %d\n",
       $ratings{1},
       $ratings{2},
       $ratings{3},
       $ratings{4},
       $ratings{5}
    );

foreach $m (sort numerically(keys(%{$movie_ratings}))) {
    $mean = compute_mean($movie_ratings->{$m});
    $var  = compute_variance($movie_ratings->{$m}, $mean);
    printf("%d:  mean = %g variance = %g rating_count = %d\n", $m, $mean, $var, scalar(@{$movie_ratings->{$m}}));
}



my ($sum) = $ratings{1} + $ratings{2} + $ratings{3} + $ratings{4} + $ratings{5};
my ($weighted_sum) = $ratings{1} + 2*$ratings{2} + 3*$ratings{3} + 4*$ratings{4} + 5*$ratings{5};
printf("Total ratings = %d\n", $sum);
my ($i);
printf("Percentage breakdown:\n");
for( $i = 1 ; $i <= 5; ++$i ) {
    printf("  %d : %g\n", $i, $ratings{$i}/$sum);
}
printf("Mean rating = %g\n", $weighted_sum / $sum);

sub numerically {
    return( $a <=> $b );
}

sub compute_mean {
    my ($arr) = @ARG;

    my ($i);
    my ($sum) = 0;
    for( $i = 0; $i < scalar(@{$arr}); ++$i ) {
        $sum += $arr->[$i];
    }

    return( $sum / scalar(@{$arr}) );
}

sub compute_variance {
    my ($arr) = @ARG;

    my ($i);
    my ($sum) = 0;
    for( $i = 0; $i < scalar(@{$arr}); ++$i ) {
        $sum += (($arr->[$i] - $mean)*($arr->[$i] - $mean));
    }

    return( $sum / scalar(@{$arr}) );
}





