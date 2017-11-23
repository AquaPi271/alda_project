#pragma once

#include <cstdint>
#include <map>
#include <vector>

struct bucket_info {
  uint32_t lower;
  uint32_t upper;
  uint32_t rating_count;
  uint32_t movie_count;  // number of movies in this bucket
};

auto compute_bulk_statistics( std::map<uint16_t, std::map<uint32_t,uint8_t>> & movies,
			      std::map<uint32_t, std::map<uint16_t,uint8_t>> & users ) -> void;
void compute_movie_ratings_counts( const std::map<uint16_t, std::map<uint32_t,uint8_t>> & movies,
				   std::map<uint16_t,uint32_t> & movies_counts,
				   uint32_t buckets );
void get_sorted_frequent_rated_movies( std::map<uint16_t,std::map<uint32_t,uint8_t>> & movies,
				       std::vector<uint16_t> & sorted_frequent_rated_movies );
void get_sorted_frequent_rated_users( std::map<uint32_t,std::map<uint16_t,uint8_t>> & users,
				      std::vector<uint32_t> & sorted_frequent_rated_users );
