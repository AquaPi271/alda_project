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
		  
auto main( int argc, char **argv ) -> int {

  if( argc != 2 ) {
    std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
    exit(1);
  }
  std::string base_path{"./"};
  std::string s{ argv[1] };

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

  // Get sorted user list of most active users.

  std::map<uint32_t,std::vector<uint32_t>> ratings_users;

  classify_users( users, ratings_users );

  std::vector<uint32_t> sorted_ratings = {};

  for( auto & ru : ratings_users ) {
    sorted_ratings.push_back( ru.first );
  }

  std::sort( sorted_ratings.begin(), sorted_ratings.end() );
  std::reverse( sorted_ratings.begin(), sorted_ratings.end() );

  for( auto & sr : sorted_ratings ) {
    std::cout << sr << " : ";
    for( auto & ru : ratings_users[sr] ) {
      std::cout << ru << " ";
    }
    std::cout << std::endl;
  }
  
  // Get sorted item list of most active items.
  
  return(0);
}
