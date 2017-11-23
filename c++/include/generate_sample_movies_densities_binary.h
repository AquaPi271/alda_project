#pragma once

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

void read_movie_file_for_ids_binary( const std::string & filename,
				     std::set<uint16_t> & movie_set,
				     std::set<uint32_t> & ids_set );
  
auto generate_sample_movies_binary( const std::string & filename,
				    std::set<uint16_t> & movie_set,
				    std::set<uint32_t> & users_set,
				    const std::string & output_filename ) -> void;
void generate_movie_user_sample_by_frequency( const std::string & directory_name,
					      uint32_t top_movie_count,
					      uint32_t top_user_count,
					      std::map<uint16_t, std::map<uint32_t,uint8_t>> & movies,
					      std::map<uint32_t, std::map<uint16_t,uint8_t>> & users,
					      std::vector<uint32_t> & sorted_frequent_rated_movies );
