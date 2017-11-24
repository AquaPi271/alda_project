#pragma once

#include <cstdint>
#include <map>
#include <vector>

struct nn {
  uint32_t user_id;
  uint32_t matched_count;
  float    dist;
};

bool find_top_knn_users_cosine( std::map<uint16_t,std::map<uint32_t,uint8_t>> & train_movie_users,
				std::map<uint32_t,std::map<uint16_t,uint8_t>> & train_user_movies,
				std::map<uint32_t,std::map<uint16_t,uint8_t>> & test_users,
				uint32_t user_id,
				uint32_t requested_movie,
				std::vector<uint32_t> & matched_user_ids);
bool find_top_knn_users_pearson( std::map<uint16_t,std::map<uint32_t,uint8_t>> & train_movie_users,
				 std::map<uint32_t,std::map<uint16_t,uint8_t>> & train_user_movies,
				 std::map<uint32_t,std::map<uint16_t,uint8_t>> & test_users,
				 uint32_t user_id,
				 uint32_t requested_movie,
				 std::vector<uint32_t> & matched_user_ids );

bool find_top_knn_users_cosine_normalized
(
 std::map<uint16_t,std::map<uint32_t,uint8_t>> & train_movie_users,
 std::map<uint32_t,std::map<uint16_t,float>> & train_user_movies,
 std::map<uint32_t,std::map<uint16_t,float>> & test_users,
 uint32_t user_id,
 uint32_t requested_movie,
 std::vector<uint32_t> & matched_user_ids
 );

void normalize_from( std::map<uint32_t,std::map<uint16_t,uint8_t>> & u_m_r,
		     std::map<uint32_t,std::map<uint16_t,float>> & u_m_r_OUT );

void add_to_knn( uint32_t k,
		 uint32_t wuser_id,
		 uint32_t matched_count,
		 float dist,
		 std::vector<nn> & knn );

float compute_user_bias( uint32_t user_id,
			 std::map<uint32_t,std::map<uint16_t,uint8_t>> & train_users,
			 std::map<uint16_t,float> train_movie_averages );
