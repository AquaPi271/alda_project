#pragma once

#include <cstdint>
#include <map>
#include <string>

const uint32_t record_size = 9;
const uint32_t buffer_size = record_size * 1024;

uint32_t read_uint32_from_buffer( char *buffer, uint32_t index );
uint16_t read_uint16_from_buffer( char *buffer, uint32_t index );
uint8_t  read_uint8_from_buffer( char *buffer, uint32_t index );
void write_uint32_to_buffer( char *buffer, uint32_t data, uint32_t index );
void write_uint16_to_buffer( char *buffer, uint16_t data, uint32_t index );
void write_uint8_to_buffer( char *buffer, uint8_t data, uint32_t index );

void read_record( char * buffer, uint32_t index,
		  uint16_t * movie,
		  uint32_t * user,
		  uint8_t  * rating,
		  uint16_t * days );
void read_movie_user_record( char * buffer, uint32_t index, uint16_t * movie, uint32_t * user );
void read_binary_to_movies_users( const std::string & filename,
				  std::map<uint16_t, std::map<uint32_t,uint8_t>> & movies,
				  std::map<uint32_t, std::map<uint16_t,uint8_t>> & users
				  );
void read_binary_to_movies( const std::string & filename,
			    std::map<uint16_t, std::map<uint32_t,uint8_t>> & movies
			    );
void read_binary_to_users( const std::string & filename,
			   std::map<uint32_t, std::map<uint16_t,uint8_t>> & users
			   );
