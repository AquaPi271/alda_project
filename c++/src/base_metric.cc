#include <algorithm>
#include <cmath>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <vector>

#include "load_data.h"

auto main( int argc, char **argv ) -> int {

  if( argc != 4 ) {
    std::cerr << "Usage: " << argv[0] << " <filename> <random_seed> <train_set_percent>" << std::endl;
    exit(1);
  }

  std::string s{ argv[1] };
  uint32_t random_seed = std::stoi( argv[2] );
  float train_set_percent = std::stof( argv[3] );

  std::default_random_engine random_generator{random_seed};
  
  std::map<uint16_t,std::map<uint32_t,uint8_t>> movies;
  std::map<uint32_t,std::map<uint16_t,uint8_t>> users;
  
  read_binary_to_movies_users( s, movies, users );

  // Spaghetti for now.

  std::vector<uint32_t> random_user_ids = {};
  for( auto & u : users ) {
    random_user_ids.push_back( u.first );
  }
  
  std::random_shuffle( random_user_ids.begin(), random_user_ids.end() );

  uint32_t train_user_count = std::round( train_set_percent * users.size() );
  uint32_t test_user_count = users.size() - train_user_count;

  auto train_start = random_user_ids.begin();
  auto train_end   = random_user_ids.begin() + train_user_count;
  auto test_start  = random_user_ids.begin() + train_user_count;
  auto test_end    = random_user_ids.end();
  
  std::vector<uint32_t> train_user_ids(train_start, train_end);
  std::vector<uint32_t> test_user_ids(test_start, test_end);

  //std::cout << "train_user_count  = " << train_user_count << std::endl;
  //std::cout << "train vector size = " << train_user_ids.size() << std::endl;
  //std::cout << "test_user_count   = " << test_user_count << std::endl;
  //std::cout << "test vector size  = " << test_user_ids.size() << std::endl;

  // Build train and test map.

  std::map<uint32_t,std::map<uint16_t,uint8_t>> train_users;
  std::map<uint32_t,std::map<uint16_t,uint8_t>> test_users;
  std::map<uint16_t,std::vector<uint8_t>> train_movies;
  std::map<uint16_t,float> train_movie_averages;
  
  for( auto & u : train_user_ids ) {
    train_users[u] = users[u];
    for( auto & m : users[u] ) {
      train_movies[m.first].push_back(m.second);
    }
  }
  for( auto & u : test_user_ids ) {
    test_users[u] = users[u];
  }
  for( auto & m : train_movies ) {
    uint32_t sum = 0;
    for( auto & r : m.second ) {
      sum += r;
    }
    train_movie_averages[m.first] = static_cast<float>(sum) / static_cast<float>( m.second.size() );
  }

  // RMSE start!

  float rmse_N = 0.0f;
  float rmse_D = 0.0f;

  for( auto & tu : test_users ) {
    auto movies_rated = tu.second.size();
    std::uniform_int_distribution<uint32_t> dist(0, movies_rated - 1);
    auto random_offset = dist(random_generator);
    auto item = tu.second.begin();
    std::advance( item, random_offset );
    //std::cout << "movie " << (*item).first << " : " << static_cast<uint32_t>((*item).second) << std::endl;
    auto test_rating = static_cast<float>((*item).second);
    auto test_movie = (*item).first;
    auto avg_rating = train_movie_averages[test_movie];
    rmse_N += ((test_rating - avg_rating)*(test_rating - avg_rating));
    ++rmse_D;
    
  }

  std::cout << "RMSE = " << sqrt( rmse_N / rmse_D ) << std::endl;
  
  return(0);

}
