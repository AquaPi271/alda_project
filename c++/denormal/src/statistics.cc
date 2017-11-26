#include "statistics.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <map>
#include <set>
#include <vector>

auto compute_bulk_statistics( std::map<uint16_t, std::map<uint32_t,uint8_t>> & movies,
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

void compute_movie_ratings_counts( const std::map<uint16_t, std::map<uint32_t,uint8_t>> & movies,
				   std::map<uint16_t,uint32_t> & movie_counts,
				   uint32_t buckets ) {

  // Stratification attempts.

  uint64_t total_ratings = 0;
  
  for( auto & m : movies ) {
    movie_counts[m.first] = m.second.size();
    total_ratings += m.second.size();
  }

  // Break into N buckets by counts.

  uint64_t bucket_size = total_ratings / buckets;

  // Construct counts based map.

  std::map<uint32_t,std::vector<uint16_t>> count_based_map;
  std::vector<uint32_t> raw_counts = {};
  
  for( uint16_t i = 0; i < movie_counts.size(); ++i ) {
    if( movie_counts[i] > 0 ) {
      count_based_map[movie_counts[i]].push_back( i );
      raw_counts.push_back(movie_counts[i]);
    } 
    //  std::cout << i << " : " << movie_counts[i] << std::endl;
  }

  std::sort( raw_counts.begin(), raw_counts.end() );

  uint32_t current_bucket_count = 0;
  uint32_t current_bucket_low = 0;
  uint32_t bucket_index = 1;
  uint32_t last_c;
  std::vector<bucket_info>bucket_vec = {};
  
  for( auto & c : raw_counts ) {
    current_bucket_count += c;
    if( current_bucket_count >= bucket_size ) {
      bucket_info current_bucket{current_bucket_low,c,current_bucket_count,0};
      bucket_vec.push_back(current_bucket);
      current_bucket_count = 0;
      current_bucket_low = c;
      ++bucket_index;
    }
    last_c = c;
  }
  bucket_info current_bucket{current_bucket_low,last_c,current_bucket_count,0};
  bucket_vec.push_back(current_bucket);

  for( auto & b : bucket_vec ) {
    uint32_t movie_ratings = 0;
    for( auto & m : movies ) {
      uint32_t current_movie_ratings = m.second.size();
      if( (current_movie_ratings > b.lower) && (current_movie_ratings <= b.upper) ) {
	++b.movie_count;
      }
    }
  }

  uint32_t total_movies = 0;

  for( bucket_index = 0; bucket_index < bucket_vec.size(); ++bucket_index ) {
    total_movies += bucket_vec[bucket_index].movie_count;
  }
  
  for( bucket_index = 0; bucket_index < bucket_vec.size(); ++bucket_index ) {
    std::cout << "Bucket " << bucket_index << " " << bucket_vec[bucket_index].lower
	      << " < bucket <= " << bucket_vec[bucket_index].upper
	      << " ( rating count = " << bucket_vec[bucket_index].rating_count << " )"
	      << " [ movies in bucket = " << bucket_vec[bucket_index].movie_count << " ]"
	      << " [ percent composition = "
	      << (static_cast<float>(bucket_vec[bucket_index].movie_count) / static_cast<float>(total_movies))
	      << " ] " << std::endl;
  }
}

void get_sorted_frequent_rated_movies( std::map<uint16_t,std::map<uint32_t,uint8_t>> & movies,
				       std::vector<uint16_t> & sorted_frequent_rated_movies ) {
  
  // Index is number of ratings which points to movie ids that have that many ratings.
  std::map<uint32_t,std::vector<uint16_t>> ratings_movies = {};  
  
  for( auto & m : movies ) {
    auto rating_count = m.second.size();
    if( ratings_movies.find(rating_count) != ratings_movies.end() ) {
      ratings_movies[rating_count].push_back( m.first );
    } else {
      ratings_movies[rating_count] = { m.first };
    }
  }
  
  std::set<uint32_t> sorted_movie_ratings = {};
  
  for( auto & rm : ratings_movies ) {
    sorted_movie_ratings.insert( rm.first );
  }

  for( auto smi = sorted_movie_ratings.rbegin(); smi != sorted_movie_ratings.rend(); ++smi ) {
    for( auto & movie_id : ratings_movies[*smi] ) {
      sorted_frequent_rated_movies.push_back( movie_id );
    }
  }
  
}

void get_sorted_frequent_rated_users( std::map<uint32_t,std::map<uint16_t,uint8_t>> & users,
				      std::vector<uint32_t> & sorted_frequent_rated_users ) {
  
  // Index is number of ratings which points to user ids that have that many ratings.
  std::map<uint32_t,std::vector<uint32_t>> ratings_users = {};
  
  for( auto & u : users ) {
    auto rating_count = u.second.size();
    if( ratings_users.find(rating_count) != ratings_users.end() ) {
      ratings_users[rating_count].push_back( u.first );
    } else {
      ratings_users[rating_count] = { u.first };
    }
  }
  
  std::set<uint32_t> sorted_user_ratings = {};
  
  for( auto & rm : ratings_users ) {
    sorted_user_ratings.insert( rm.first );
  }

  for( auto sui = sorted_user_ratings.rbegin(); sui != sorted_user_ratings.rend(); ++sui ) {
    for( auto & user_id : ratings_users[*sui] ) {
      sorted_frequent_rated_users.push_back( user_id );
    }
  }
}
