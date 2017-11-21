#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <set>
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
void     read_movie_file_binary( const std::string & filename,
				 std::set<uint16_t> & movie_set,
				 std::set<uint32_t> & users_set,
				 const std::string & output_filename );
void read_movie_user_record( char * buffer, uint32_t index, uint16_t * movie, uint32_t * user );
void read_movie_file_for_ids_binary( const std::string & filename,
				     std::set<uint16_t> & movie_set,
				     std::set<uint32_t> & ids_set );
void write_uint32_to_buffer( char *buffer, uint32_t data, uint32_t index );
void write_uint16_to_buffer( char *buffer, uint16_t data, uint32_t index );
void write_uint8_to_buffer( char *buffer, uint8_t data, uint32_t index );
  
const uint32_t record_size = 9;
const uint32_t buffer_size = record_size * 1024;
char buffer[buffer_size];
char out_buffer[buffer_size];

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
			     std::set<uint16_t> & movie_set,
			     std::set<uint32_t> & users_set,
			     const std::string & output_filename ) -> void {

  std::ifstream infile;
  std::ofstream outfile;
  uint32_t bytes_read = 0;
  uint32_t file_size = 0;
  uint16_t movie;
  uint32_t user;
  uint8_t rating;
  uint16_t days;
  uint32_t count = 0;
  uint32_t index = 0;
  uint32_t bytes_in_buffer = 0;
  uint32_t bytes_to_write = record_size;
  
  infile.open( filename, std::ios::binary | std::ios::in | std::ios::ate );
  if( !infile.is_open() ) {
    std::cout << "Could not open file " << filename << std::endl;
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
      if( movie_set.find(movie) == movie_set.end() ) {
	continue;
      }
      if( users_set.find(user) == users_set.end() ) {
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

void read_movie_user_record( char * buffer, uint32_t index, uint16_t * movie, uint32_t * user ) {
  *movie = read_uint16_from_buffer( buffer, index );
  *user  = read_uint32_from_buffer( buffer, index+2 );
}

auto read_movie_file_for_ids_binary( const std::string & filename,
				     std::set<uint16_t> & movie_set,
				     std::set<uint32_t> & ids_set ) -> void {

  uint32_t bytes_read = 0;
  uint32_t file_size = 0;
  uint16_t movie;
  uint32_t user;
  std::ifstream infile;

  uint32_t count = 0;
  
  infile.open( filename, std::ios::binary | std::ios::in | std::ios::ate );
  if( !infile.is_open() ) {
    std::cout << "Could not open file " << filename << std::endl;
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
      read_movie_user_record( buffer, r * record_size, &movie, &user );
      if( movie_set.find(movie) == movie_set.end() ) {
	continue;
      }
      ids_set.insert( user );
    }
  }
  infile.close();  
}

void write_uint32_to_buffer( char *buffer, uint32_t data, uint32_t index ) {
  buffer[index] = data & 0xFF;
  buffer[index+1] = (data >> 8) & 0xFF;
  buffer[index+2] = (data >> 16) & 0xFF;
  buffer[index+3] = (data >> 24) & 0xFF;
}

void write_uint16_to_buffer( char *buffer, uint16_t data, uint32_t index ) {
  buffer[index] = data & 0xFF;
  buffer[index+1] = (data >> 8) & 0xFF;
}

void write_uint8_to_buffer( char *buffer, uint8_t data, uint32_t index ) {
  buffer[index] = data & 0xFF;
}

auto main( int32_t argc, char **argv ) -> int {
  
  // Awesome, R can handle the overall sampling.  Just need to supply the data.
  
  if( argc != 5 ) {
    std::cerr << "Usage:  " << argv[0] << " <seed> <movie_count> <user_count> <output_filename>" << std::endl;
    exit(1);
  }

  std::string movie_filename{ "ALL_MOVIES_BINARY" };
  std::string seed_text{ argv[1] };
  std::string movie_count_text{ argv[2] };
  std::string user_count_text{ argv[3] };
  std::string output_filename{ argv[4] };
  
  uint32_t seed = std::stoi( seed_text );
  std::srand( seed );
  uint32_t movie_count = std::stoi( movie_count_text );
  uint32_t user_count = std::stoi( user_count_text );

  if( movie_count < 1 || movie_count > 17700 ) {
    std::cerr << "Error:  Movie count must be between 1 and 17700 but is " <<
      movie_count << std::endl;
    exit(1);
  }
  if( user_count < 1 || user_count > 480189 ) {
    std::cerr << "Error:  User count must be between 1 and 480189 but is " <<
      user_count << std::endl;
    exit(1);
  }

  std::vector<uint16_t> movies_selected = {};
  for( auto i = 1; i <= 17700; ++i ) {
    movies_selected.push_back(i);
  }

  std::random_shuffle(movies_selected.begin(), movies_selected.end());
  std::set<uint16_t> random_movies = {};
  for( auto i = 1; i <= movie_count; ++i ) {
    random_movies.insert( movies_selected[i] );
  }
  
  std::set<uint32_t> users = {};
  read_movie_file_for_ids_binary(movie_filename, random_movies, users);

  std::vector<uint32_t> valid_user_ids = {};
  for( auto & u : users ) {
    valid_user_ids.push_back(u);
  }

  std::random_shuffle(valid_user_ids.begin(), valid_user_ids.end());

  std::set<uint32_t> users_selected = {};
  for( auto i = 1; i <= user_count; ++i ) {
    users_selected.insert( valid_user_ids[i] );
  }
  
  read_movie_file_binary( movie_filename, random_movies, users_selected, output_filename );
  
  return(0);
}
