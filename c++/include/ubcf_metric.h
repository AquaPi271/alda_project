#pragma once

#include <cstdint>
#include <map>

struct nn {
  uint32_t user_id;
  uint32_t matched_count;
  float    dist;
};

bool find_top_knn_users_cosine_normalized
( uint32_t k,
  std::map<uint16_t,std::map<uint32_t,uint8_t>> & train_movie_users,
  std::map<uint32_t,std::map<uint16_t,float>> & train_user_movies,
  std::map<uint32_t,std::map<uint16_t,float>> & test_users,
  uint32_t user_id,
  uint32_t requested_movie,
  std::vector<uint32_t> & matched_user_ids );
bool find_top_knn_users_pearson_normalized
( uint32_t k,
  std::map<uint16_t,std::map<uint32_t,uint8_t>> & train_movie_users,
  std::map<uint32_t,std::map<uint16_t,float>> & train_user_movies,
  std::map<uint32_t,std::map<uint16_t,float>> & test_users,
  uint32_t user_id,
  uint32_t requested_movie,
  std::vector<uint32_t> & matched_user_ids );
void add_to_knn( uint32_t k,
		 uint32_t wuser_id,
		 uint32_t matched_count,
		 float dist,
		 std::vector<nn> & knn );
void normalize_from( std::map<uint32_t,std::map<uint16_t,uint8_t>> & u_m_r,
		     std::map<uint32_t,std::map<uint16_t,float>> & u_m_r_OUT,
		     bool normalize );
float compute_user_bias( uint32_t user_id,
			 std::map<uint32_t,std::map<uint16_t,uint8_t>> & train_users,
			 std::map<uint16_t,float> train_movie_averages,
			 bool skip_test_movie,
			 uint32_t test_movie );
void extract_fold_user_ids( const std::vector<uint32_t> & random_user_ids,
			    uint32_t fold,
			    uint32_t total_folds,
			    std::vector<uint32_t> & train_user_ids,
			    std::vector<uint32_t> & test_user_ids );
void extract_fold_user_ids_percentage_method( const std::vector<uint32_t> & random_user_ids,
					      float train_set_percent,
					      std::vector<uint32_t> & train_user_ids,
					      std::vector<uint32_t> & test_user_ids );
