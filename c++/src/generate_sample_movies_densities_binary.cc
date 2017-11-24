#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "generate_sample_movies_densities_binary.h"
#include "load_data.h"
#include "statistics.h"

void generate_sample_from_movie_user_pairs( const std::string & movie_filename,
					    const std::string & output_filename,
					    std::map<uint16_t,std::set<uint32_t>> & movie_users_to_extract ) {

  std::ifstream infile;
  char          buffer[buffer_size];
  std::ofstream outfile;
  char          out_buffer[buffer_size];
  uint32_t      bytes_read = 0;
  uint32_t      file_size = 0;
  uint16_t      movie;
  uint32_t      user;
  uint8_t       rating;
  uint16_t      days;
  uint32_t      count = 0;
  uint32_t      index = 0;
  uint32_t      bytes_in_buffer = 0;
  uint32_t      bytes_to_write = record_size;
  
  infile.open( movie_filename, std::ios::binary | std::ios::in | std::ios::ate );
  if( !infile.is_open() ) {
    std::cout << "Could not open file " << movie_filename << std::endl;
    exit(1);
  }
  outfile.open( output_filename, std::ios::binary | std::ios::out );
  if( !outfile.is_open() ) {
    std::cout << "Could not create output file " << output_filename << std::endl;
    exit(1);
  }
  
  file_size = infile.tellg();
  infile.seekg(0, std::ios::beg);
  while( bytes_read < file_size ) {
    uint32_t amount_read = infile.readsome( buffer, buffer_size );
    bytes_read += amount_read;
    if( amount_read % 9 != 0 ) {
      std::cerr << "Badness has happened.  Non-multiple of record size" << std::endl;
      exit(1);
    }
    uint32_t records_read = amount_read / record_size;
    for( uint32_t r = 0; r < records_read; ++r ) {
      ++count;
      if( count % 1000000 == 0 ) {
	std::cerr << count << std::endl;
      }
      read_record( buffer, r * record_size, &movie, &user, &rating, &days );
      if( movie_users_to_extract.find(movie) == movie_users_to_extract.end() ) {
	continue;
      }
      if( movie_users_to_extract[movie].find(user) == movie_users_to_extract[movie].end() ) {
	continue;
      }

      if( bytes_to_write + bytes_in_buffer > buffer_size ) {
	outfile.write( out_buffer, bytes_in_buffer );
	bytes_in_buffer = 0;
	index = 0;
      }
      write_uint16_to_buffer( out_buffer, movie, index );
      write_uint32_to_buffer( out_buffer, user, index+2 );
      write_uint8_to_buffer( out_buffer, rating, index+6 );
      write_uint16_to_buffer( out_buffer, days, index+7 );
      index += bytes_to_write;
      bytes_in_buffer += bytes_to_write;
    }
  }  
  if( bytes_in_buffer > 0 ) {
    outfile.write( out_buffer, bytes_in_buffer );
  }
  infile.close();
  outfile.close();
}

void extract_movie_user_sample_by_frequency( uint32_t top_movie_count,
					     uint32_t top_user_count,
					     std::map<uint16_t, std::map<uint32_t,uint8_t>> & movies,
					     std::map<uint32_t, std::map<uint16_t,uint8_t>> & users,
					     std::vector<uint16_t> & sorted_frequent_rated_movies,
					     std::map<uint16_t,std::set<uint32_t>> & movie_users_to_extract) {

  // Algorithm (should be straightforward) :
  //   Already have sorted frequent movies.
  //   Check that X >= sorted_frequent_rated_movies.
  //   Get a list of X top rated movies and place into a set. (A)
  //   Scroll through set, for each movie:
  //     Get all users and place into a set.
  //   From available user set, see if >= top_user_count to ensure we can extract that many.
  //   Build subset of users map of the movies in this set from master list.
  //   Get sorted frequent user list from the above map.
  //   Place top Y users into a set from prior step.  (B)
  //   Foreach frequent movie in set A
  //     Grab all users in B that rated A.
  //     Store them to outputfile.

  if( top_movie_count > sorted_frequent_rated_movies.size() ) {
    std::cerr << "Not able to extract " << top_movie_count << " movies from the frequent list." << std::endl;
    exit( 1 );
  }

  std::set<uint16_t> selected_movies_set{ sorted_frequent_rated_movies.begin(),
                                          sorted_frequent_rated_movies.begin() + top_movie_count };
  
  std::set<uint32_t> selected_users_set = {};
  for( auto & sel_movie : selected_movies_set ) {
    for( auto & u_r : movies[sel_movie] ) {
      selected_users_set.insert( u_r.first );
    }
  }
  
  if( top_user_count > selected_users_set.size() ) {
    std::cerr << "Not able to extract " << top_user_count << " users from the frequent list." << std::endl;
    exit( 1 );
  }

  std::map<uint32_t,std::map<uint16_t,uint8_t>> frequent_users_subset;
  for( auto & sel_u : selected_users_set ) {
    frequent_users_subset[sel_u] = {};
    for( auto & m_r : users[sel_u] ) {
      if( selected_movies_set.find( m_r.first ) != selected_movies_set.end() ) {
	frequent_users_subset[sel_u][m_r.first] = m_r.second;
      }
    }
  }
  
  std::vector<uint32_t> sorted_frequent_rated_users_final = {};
  get_sorted_frequent_rated_users( frequent_users_subset, sorted_frequent_rated_users_final );

  std::set<uint32_t> top_selected_users_set = { sorted_frequent_rated_users_final.begin(),
						sorted_frequent_rated_users_final.begin() + top_user_count };

  movie_users_to_extract = {};

  for( auto & fm : selected_movies_set ) {
    movie_users_to_extract[fm] = {};
    for( auto & u_r : movies[fm] ) {
      if( top_selected_users_set.find(u_r.first) != top_selected_users_set.end() ) {
	movie_users_to_extract[fm].insert( u_r.first );
      }
    }
  }

  // Whew!  Now re-read movie file and only output movie/user pairs found in movie_users_to_extract list.

}

auto main( int32_t argc, char **argv ) -> int {
  
  // Awesome, R can handle the overall sampling.  Just need to supply the data.
  
  if( argc != 2 ) {
    std::cerr << "Usage:  " << argv[0] << " <output_directory>" << std::endl;
    exit(1);
  }

  // Find movies with most rankings.   X
  // Find users with most rankings.    Y
  // For this to work Y is dependent on X.  So get X first then start at the top of the list for Y.
  // Each item in Y must be in one of X.  If it is, grab it.  If it isn't skip.

  // Second approach is to get top X movies.  Then from these movies, find the most frequent users who rate.
  // This seems like exactly what I need, instead of the first method.

  std::string movie_filename{ "ALL_MOVIES_BINARY" };
  std::string output_directory{ argv[1] };
  
  std::map<uint16_t, std::map<uint32_t,uint8_t>> movies;
  std::map<uint32_t, std::map<uint16_t,uint8_t>> users;

  std::vector<uint16_t> sorted_frequent_rated_movies = {};
  std::vector<uint32_t> sorted_frequent_rated_users = {};
  
  read_binary_to_movies_users( movie_filename, movies, users );
  std::cerr << "Sorting movies..." << std::endl;
  get_sorted_frequent_rated_movies( movies, sorted_frequent_rated_movies );
  std::cerr << "Sorting users..." << std::endl;
  get_sorted_frequent_rated_users( users, sorted_frequent_rated_users );

  // Got the frequent lists, now output combinations of X and Y to files.

  std::cerr << "Extracting top movies / users..." << std::endl;
  
  for( uint32_t top_movie_count = 1000; top_movie_count < 10500; top_movie_count += 2000 ) {
    for( uint32_t top_user_count = 500; top_user_count < 5500; top_user_count += 1000 ) {

      std::cout << ">>> Extracting movie count / user count = " << top_movie_count << " / " << top_user_count << std::endl;
      
  
      std::map<uint16_t,std::set<uint32_t>>  movie_users_to_extract = {};

      extract_movie_user_sample_by_frequency( top_movie_count, top_user_count,
					      movies, users,
					      sorted_frequent_rated_movies,
					      movie_users_to_extract );
      
      std::string output_filename = output_directory + "/TSAMPLE_M" +
	std::to_string(top_movie_count) + "_U" + std::to_string(top_user_count);
      
      std::cerr << "Generating sample file " << output_filename << "..." << std::endl;
      
      generate_sample_from_movie_user_pairs( movie_filename,
					     output_filename,
					     movie_users_to_extract );
    }
  }
  
  return(0);
}
