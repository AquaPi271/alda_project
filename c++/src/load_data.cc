#include "load_data.h"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <map>
#include <string>


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

void read_movie_user_record( char * buffer, uint32_t index, uint16_t * movie, uint32_t * user ) {
  *movie = read_uint16_from_buffer( buffer, index );
  *user  = read_uint32_from_buffer( buffer, index+2 );
}

auto read_binary_to_movies_users( const std::string & filename,
				  std::map<uint16_t, std::map<uint32_t,uint8_t>> & movies,
				  std::map<uint32_t, std::map<uint16_t,uint8_t>> & users
				  ) -> void {
  
  std::ifstream infile;
  char          buffer[buffer_size];
  uint32_t      bytes_read = 0;
  uint16_t      movie;
  uint32_t      user;
  uint8_t       rating;
  uint16_t      days;
  uint32_t      count = 0;
  
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

auto read_binary_to_movies( const std::string & filename,
			    std::map<uint16_t, std::map<uint32_t,uint8_t>> & movies
			    ) -> void {
  
  std::ifstream infile;
  char          buffer[buffer_size];
  uint32_t      bytes_read = 0;
  uint16_t      movie;
  uint32_t      user;
  uint8_t       rating;
  uint16_t      days;
  uint32_t      count = 0;
  
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
      ++count;
      if( count % 1000000 == 0 ) {
	std::cerr << count << std::endl;
      }
    }
  } 
  infile.close();
}

auto read_binary_to_movies_users( const std::string & filename,
				    std::map<uint32_t, std::map<uint16_t,uint8_t>> & users
				    ) -> void {

  std::ifstream infile;
  char          buffer[buffer_size];
  uint32_t      bytes_read = 0;
  uint16_t      movie;
  uint32_t      user;
  uint8_t       rating;
  uint16_t      days;

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
      users[user][movie] = rating;
      ++count;
      if( count % 1000000 == 0 ) {
	std::cerr << count << std::endl;
      }
    }
  } 
  infile.close();
}
