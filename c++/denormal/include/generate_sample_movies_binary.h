#pragma once

#include <cstdint>
#include <set>
#include <string>

void read_movie_file_for_ids_binary( const std::string & filename,
				     std::set<uint16_t> & movie_set,
				     std::set<uint32_t> & ids_set );
  
auto generate_sample_movies_binary( const std::string & filename,
				    std::set<uint16_t> & movie_set,
				    std::set<uint32_t> & users_set,
				    const std::string & output_filename ) -> void;
