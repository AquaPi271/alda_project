#!/usr/bin/perl

use English;
use strict;
use List::Util qw/shuffle/;

my ($dataset) = $ARGV[0];
my ($line);
my ($users) = {};
my ($user_count) = 0;

open( DATASET, "<".$dataset ) || die( "Failed to open '".$dataset."'" );
while( $line = <DATASET> ) {
    chomp( $line );
    my (@data) = split(/,/,$line);
    if( !exists(${$users}{$data[1]}) ) {
	++$user_count;
    }
    $users->{$data[1]}->{$data[0]} = $data[2];
}
close( DATASET );

#print("user count = ".$user_count."\n");

my ($train_users) = {};
my ($train_user_count) = int($user_count * 0.7);
my ($test_users) = {};
my ($test_user_count) = $user_count - $train_user_count;

#print("train user count = ".$train_user_count."\n");
#print("test user count  = ".$test_user_count."\n");

# How to make the test and training datasets?
# Do UBCF first but only on averages.  Select X users for training and Y users for test.
# For test set randomly remove 1 reading.

my (@random_user_ids) = shuffle(keys(%{$users}));
my (@train_user_ids) = @random_user_ids[0..($train_user_count-1)];
my (@test_user_ids) = @random_user_ids[$train_user_count..($train_user_count+$test_user_count-1)];

my ($id);
my ($movie);

foreach $id (@train_user_ids) {
    foreach $movie (keys(%{$users->{$id}})) {
	$train_users->{$id}->{$movie} = $users->{$id}->{$movie};
    }
}

foreach $id (@test_user_ids) {
    foreach $movie (keys(%{$users->{$id}})) {
	$test_users->{$id}->{$movie} = $users->{$id}->{$movie};
    }
}

my ($train_movies) = {};

foreach $id (keys(%{$train_users})) {
    foreach $movie (keys(%{$train_users->{$id}})) {
	if( exists(${$train_movies}{$movie}) ) {
	    push(@{$train_movies->{$movie}->{"ratings"}}, $train_users->{$id}->{$movie});
	} else {
	    $train_movies->{$movie}->{"ratings"} = [$train_users->{$id}->{$movie}];
	}
    }
}

my ($train_movies_users) = {};

foreach $id (keys(%{$train_users})) {
    foreach $movie (keys(%{$train_users->{$id}})) {
	$train_movies_users->{$movie}->{$id} = $train_users->{$id}->{$movie};
    }
}

# (4 + 8 + 12)/(sqrt(4 + 16 + 9)sqrt(4+4+16) = 24 / (sqrt(29)sqrt(24) = 

# TODO:  Handle the case of 0 ratings!

my ($rating);
foreach $movie (keys(%{$train_movies})) {
    my ($sum) = 0;
    foreach $rating (@{$train_movies->{$movie}->{"ratings"}}) {
	$sum += $rating;
    }
    $train_movies->{$movie}->{"average"} = $sum / scalar(@{$train_movies->{$movie}->{"ratings"}});
#    printf("Movie %d:  average = %g  ratings = %d\n", 
#	   $movie, 
#	   $train_movies->{$movie}->{"average"},
#	   scalar(@{$train_movies->{$movie}->{"ratings"}}));
}

# From the test user set go through each user and randomly select a movie they did not rate.  Use
# average value from the train set as the actual movie value.  Start recording RMSE.

my ($rmse_N) = 0;
my ($rmse_D) = 0;
my ($count) = 0;
foreach $id (keys(%{$test_users})) {
    ++$count;
    if( $count % 100 == 0 ) { print($count."\n"); }
    my ($movies_rated) = scalar(keys(%{$test_users->{$id}}));
    # Select one at random to predict rating.
    my ($m_select) = int(rand()*$movies_rated);
    my ($request_movie) = (keys(%{$test_users->{$id}}))[$m_select];
    my ($test_rating) = $test_users->{$id}->{$request_movie};
    find_top_knn_users( $train_movies_users, $test_users, $id, $request_movie );
    #my ($avg_rating) = $train_movies->{$request_movie}->{"average"};
    #$rmse_N += (($test_rating - $avg_rating)**2);
    #$rmse_D++;
}
#print("RMSE = ".sqrt($rmse_N/$rmse_D)."\n");
    

sub find_top_knn_users {

    my ($train_movies_users, $test_users, $test_user_id, $requested_movie) = @ARG;

    # Find all users that rated the requested movie.

    my ($id);
    #my (@users_who_rated) = ();

    # Compute similarity:
    #@users_who_rated = keys(%{$train_movies_users->{$requested_movie}});


    my ($a,$b,$AB,$A2,$B2,$user_who_rated,$test_user_movie);

    
    foreach $user_who_rated (keys(%{$train_movies_users->{$requested_movie}})) {
	$AB = 0;
	$A2 = 0;
	$B2 = 0;
#	printf("Examining user ".$user_who_rated.", ");
	foreach $test_user_movie (keys(%{$test_users->{$test_user_id}})) {
	    $a = $test_users->{$test_user_id}->{$test_user_movie};
	    if( $test_user_movie != $requested_movie ) {
		if( exists( ${$train_movies_users->{$test_user_movie}}{$user_who_rated} ) ) {
		    $b = $train_movies_users->{$test_user_movie}->{$user_who_rated};
#		    print( "matched movie ".$test_user_movie.":  a = ".$a." b = ".$b."\n");
		    $AB += ($a * $b);
		    $A2 += ($a * $a);
		    $B2 += ($b * $b);		    
		}
	    }
	}
	my ($D) = sqrt($A2) * sqrt($B2);
	if ($D != 0) {
#	    print($AB/(sqrt($A2)*sqrt($B2))."\n");
	}
	
    }
#    exit(1);
}
