#include <cmath>
#include <iostream>
#include <map>
#include <string>

#include "load_data.h"
#include "statistics.h"

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
  compute_bulk_statistics( movies, users );

  for( uint32_t buckets = 1; buckets < 11; ++buckets ) {
    std::cout << std::endl;
    std::cout << ">>> Buckets = " << buckets << std::endl;
    compute_movie_ratings_counts( movies, movie_counts, buckets );
  }
  
  return(0);
}
