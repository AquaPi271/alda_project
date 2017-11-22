#include "statistics.h"

#include <cmath>
#include <cstdint>
#include <iostream>
#include <map>

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
  
  for( auto & u : users ) {
    auto & movie_map = users[u.first];
    auto ratings_count = 0;
    for( auto & m : movie_map ) {
      ++ratings_count;
    }
    double diff = (average_ratings_per_user - static_cast<float>(ratings_count));
    double diff2 = diff * diff;
    sd_comp_sum += diff2;
  }
  
  std::cout << "Average number of ratings per user     = " << average_ratings_per_user << std::endl;
  std::cout << "Total number of users                  = " << user_count << std::endl;
  std::cout << "Standard deviation of ratings per user = " << sqrt(sd_comp_sum / static_cast<float>(user_count)) << std::endl; 

  
}

  
