#include <algorithm>
#include <cmath>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

void read_record( char * buffer, uint32_t index,
		  uint16_t * movie,
		  uint32_t * user,
		  uint8_t  * rating,
		  uint16_t * days );
uint32_t read_uint32_from_buffer( char *buffer, uint32_t index );
uint16_t read_uint16_from_buffer( char *buffer, uint32_t index );
uint8_t  read_uint8_from_buffer( char *buffer, uint32_t index );
void read_movie_file_binary( const std::string & filename,
			     std::map<uint16_t, std::map<uint32_t,uint8_t>> & movies,
			     std::map<uint32_t, std::map<uint16_t,uint8_t>> & users
			     );

const uint32_t record_size = 9;
const uint32_t buffer_size = record_size * 1024;
char buffer[buffer_size];

void read_record( char * buffer, uint32_t index,
		  uint16_t * movie,
		  uint32_t * user,
		  uint8_t  * rating,
		  uint16_t * days ) {
  *movie  = read_uint16_from_buffer( buffer, index );
  *user   = read_uint32_from_buffer( buffer, index+2 );
  *rating = read_uint8_from_buffer(  buffer, index+6 );
  *days   = read_uint16_from_buffer( buffer, index+7 );
}

uint32_t read_uint32_from_buffer( char *buffer, uint32_t index ) {
  uint32_t data =
    static_cast<uint8_t>(buffer[index]) |
    static_cast<uint8_t>(buffer[index+1]) << 8 |
    static_cast<uint8_t>(buffer[index+2]) << 16 |
    static_cast<uint8_t>(buffer[index+3]) << 24;
  return( data );
}

uint16_t read_uint16_from_buffer( char *buffer, uint32_t index ) {
  uint16_t data =
    static_cast<uint8_t>(buffer[index]) |
    static_cast<uint8_t>(buffer[index+1]) << 8;
  return( data );
}

uint8_t read_uint8_from_buffer( char *buffer, uint32_t index ) {
  uint8_t data =
    static_cast<uint8_t>(buffer[index]);
  return( data );
}

auto read_movie_file_binary( const std::string & filename,
			     std::map<uint16_t, std::map<uint32_t,uint8_t>> & movies,
			     std::map<uint32_t, std::map<uint16_t,uint8_t>> & users
			     ) -> void {
  
  std::ifstream infile;
  uint32_t bytes_read = 0;
  uint16_t movie;
  uint32_t user;
  uint8_t rating;
  uint16_t days;

  uint32_t count = 0;
  
  infile.open( filename, std::ios::binary | std::ios::in | std::ios::ate );
  if( !infile.is_open() ) {
    std::cout << "Could not open file " << filename << std::endl;
    exit(1);
  }
  uint64_t file_size = infile.tellg();
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
      read_record( buffer, r * record_size, &movie, &user, &rating, &days );
      movies[movie][user] = rating;
      users[user][movie] = rating;
      ++count;
      if( count % 1000000 == 0 ) {
	std::cerr << count << std::endl;
      }
    }
  } 
  infile.close();
}

auto compute_statistics( std::map<uint16_t, std::map<uint32_t,uint8_t>> & movies,
			 std::map<uint32_t, std::map<uint16_t,uint8_t>> & users ) -> void {

  // Overall average ratings.

  uint32_t ratings[6] = {0,0,0,0,0,0};
  uint32_t ratings_count = 0;
  
  for( auto & m : movies ) {
    auto & user_map = movies[m.first];
    for( auto & u : user_map ) {
      ++ratings_count;
      ++ratings[u.second];
    }
  }

  std::cout << "Total number of ratings = " << ratings_count << std::endl;
  std::cout << "Ratings composition:" << std::endl;
  for( auto i = 1; i <= 5; ++i ) {
    float comp = static_cast<float>(ratings[i]) / static_cast<float>(ratings_count);
    std::cout << "   " << i << ":  " << comp << std::endl;
  }

  uint32_t user_count = 0;
  double sd_comp_sum = 0;
  
  for( auto & u : users ) {
    ++user_count;
  }

  float average_ratings_per_user = static_cast<float>(ratings_count) / static_cast<float>(user_count);

  uint32_t pure_cold_start_count = 0;
  
  for( auto & u : users ) {
    auto & movie_map = users[u.first];
    auto ratings_count = 0;
    for( auto & m : movie_map ) {
      ++ratings_count;
    }
    if( ratings_count == 1 ) {
      ++pure_cold_start_count;
    }
    double diff = (average_ratings_per_user - static_cast<float>(ratings_count));
    double diff2 = diff * diff;
    sd_comp_sum += diff2;
  }
  
  std::cout << "Average number of ratings per user     = " << average_ratings_per_user << std::endl;
  std::cout << "Total number of users                  = " << user_count << std::endl;
  std::cout << "Standard deviation of ratings per user = " << sqrt(sd_comp_sum / static_cast<float>(user_count)) << std::endl;
  std::cout << "Pure cold start count                  = " << pure_cold_start_count << std::endl;
  std::cout << "Pure cold start user rate              = " << static_cast<float>(pure_cold_start_count) / static_cast<float>(user_count) << std::endl;

  
}

  

auto main( int argc, char **argv ) -> int {

  if( argc != 2 ) {
    std::cerr << "Usage: read_netflix <filename>" << std::endl;
    exit(1);
  }
  
  std::vector<std::string> file_list = {};
  std::string base_path{"./"};
  std::string s{ argv[1] };

  std::map<uint16_t,std::map<uint32_t,uint8_t>> movies;
  std::map<uint32_t,std::map<uint16_t,uint8_t>> users;
  
  read_movie_file_binary( s, movies, users );
  std::cerr << "Movie file in memory.... Computing statistics..." << std::endl;
  compute_statistics( movies, users );
  
  return(0);

}
