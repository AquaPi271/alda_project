#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "load_data.h"
#include "statistics.h"

void classify_users( std::map<uint32_t,std::map<uint16_t,uint8_t>> & users,
		     std::map<uint32_t,std::vector<uint32_t>> & ratings_users ) {

  for( auto & u : users ) {
    auto rating_count = u.second.size();
    if( ratings_users.find(rating_count) != ratings_users.end() ) {
      ratings_users[rating_count].push_back( u.first );
    } else {
      ratings_users[rating_count] = { u.first };
    }
  }
  
}
	
void classify_movies( std::map<uint16_t,std::map<uint32_t,uint8_t>> & movies,
		      std::map<uint32_t,std::vector<uint16_t>> & ratings_movies ) {

  for( auto & m : movies ) {
    auto rating_count = m.second.size();
    if( ratings_movies.find(rating_count) != ratings_movies.end() ) {
      ratings_movies[rating_count].push_back( m.first );
    } else {
      ratings_movies[rating_count] = { m.first };
    }
  }
  
}

void display_sorted_rank_lists( std::map<uint16_t,std::map<uint32_t,uint8_t>> & movies,
				std::map<uint32_t,std::map<uint16_t,uint8_t>> & users ) {
 // Get sorted user list of most active users.

  std::map<uint32_t,std::vector<uint32_t>> ratings_users;
  std::map<uint32_t,std::vector<uint16_t>> ratings_movies;

  classify_users( users, ratings_users );
  classify_movies( movies, ratings_movies );

  std::vector<uint32_t> sorted_user_ratings = {};

  for( auto & ru : ratings_users ) {
    sorted_user_ratings.push_back( ru.first );
  }

  std::sort( sorted_user_ratings.begin(), sorted_user_ratings.end() );
  std::reverse( sorted_user_ratings.begin(), sorted_user_ratings.end() );

  for( auto & sr : sorted_user_ratings ) {
    std::cout << sr << " : ";
    for( auto & ru : ratings_users[sr] ) {
      std::cout << ru << " ";
    }
    std::cout << std::endl;
  }
  
  std::vector<uint32_t> sorted_movie_ratings = {};

  for( auto & rm : ratings_movies ) {
    sorted_movie_ratings.push_back( rm.first );
  }

  std::sort( sorted_movie_ratings.begin(), sorted_movie_ratings.end() );
  std::reverse( sorted_movie_ratings.begin(), sorted_movie_ratings.end() );

  for( auto & sr : sorted_movie_ratings ) {
    std::cout << sr << " : ";
    for( auto & ru : ratings_movies[sr] ) {
      std::cout << ru << " ";
    }
    std::cout << std::endl;
  }

}

auto main( int argc, char **argv ) -> int {

  if( argc != 3 ) {
    std::cerr << "Usage: " << argv[0] << " <filename> <density_file_only>" << std::endl;
    exit(1);
  }
  std::string base_path{"./"};
  std::string s{ argv[1] };
  uint32_t df_only = std::stoi( argv[2] );

  std::map<uint16_t,std::map<uint32_t,uint8_t>> movies;
  std::map<uint32_t,std::map<uint16_t,uint8_t>> users;
  std::map<uint16_t,uint32_t> movie_counts;
  
  read_binary_to_movies_users( s, movies, users );
  std::cerr << "Movie file in memory.... Computing statistics..." << std::endl;
  //compute_bulk_statistics( movies, users );
  //
  //for( uint32_t buckets = 1; buckets < 11; ++buckets ) {
  //  std::cout << std::endl;
  //  std::cout << ">>> Buckets = " << buckets << std::endl;
  //  compute_movie_ratings_counts( movies, movie_counts, buckets );
  //}

  //display_sorted_rank_lists( movies, users );
  
  // Compute the density.

  uint64_t total_movie_count = movies.size();
  uint64_t total_user_count = users.size();
  uint64_t total_possible_ratings = total_movie_count * total_user_count;
  uint64_t total_ratings_count = 0;

  for( auto & m : movies ) {
    total_ratings_count += m.second.size();
  }

  float density = static_cast<float>(total_ratings_count) / static_cast<float>(total_possible_ratings);

  if( df_only != 1 ) {
    std::cout << "movie count               = " << total_movie_count << std::endl;
    std::cout << "user count                = " << total_user_count << std::endl;
    std::cout << "total possible ratings    = " << total_possible_ratings << std::endl;
    std::cout << "total actual ratings      = " << total_ratings_count << std::endl;
    std::cout << "density                   = " << density << std::endl;
  } else {
    std::cout << s << " " << density << std::endl;
  }

  
  

  
  // Get sorted item list of most active items.
  
  return(0);
}
