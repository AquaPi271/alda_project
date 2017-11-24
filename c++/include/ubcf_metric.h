#pragma once

#include <cstdint>
#include <map>

bool find_top_knn_users_cosine( std::map<uint16_t,std::map<uint32_t,uint8_t>> & train_movie_users,
				std::map<uint32_t,std::map<uint16_t,uint8_t>> & train_users_movies,
				std::map<uint32_t,std::map<uint16_t,uint8_t>> & test_users,
				uint32_t user_id,
				uint32_t requested_movie );
bool find_top_knn_users_pearson( std::map<uint16_t,std::map<uint32_t,uint8_t>> & train_movie_users,
				 std::map<uint32_t,std::map<uint16_t,uint8_t>> & train_user_movies,
				 std::map<uint32_t,std::map<uint16_t,uint8_t>> & test_users,
				 uint32_t user_id,
				 uint32_t requested_movie,
				 std::vector<uint32_t> & matched_user_ids );
