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
#include "ubcf_metric.h"

struct nn {
  uint32_t user_id;
  uint32_t matched_count;
  float    cos_dist;
};

uint32_t Gk = 1;
float Gt = 0.95;

void add_to_knn_pearson( uint32_t wuser_id, uint32_t matched_count, float cos_dist, std::vector<nn> & knn );
void add_to_knn( uint32_t wuser_id, uint32_t matched_count, float cos_dist, std::vector<nn> & knn );
bool find_top_knn_users_cosine_record_debug( std::map<uint16_t,std::map<uint32_t,uint8_t>> & train_movie_users,
					     std::map<uint32_t,std::map<uint16_t,uint8_t>> & train_user_movies,
					     std::map<uint32_t,std::map<uint16_t,uint8_t>> & test_users,
					     uint32_t user_id,
					     uint32_t requested_movie,
					     std::vector<uint32_t> & matched_user_ids);

bool find_top_knn_users_cosine_record( std::map<uint16_t,std::map<uint32_t,uint8_t>> & train_movie_users,
				       std::map<uint32_t,std::map<uint16_t,uint8_t>> & train_user_movies,
				       std::map<uint32_t,std::map<uint16_t,uint8_t>> & test_users,
				       uint32_t user_id,
				       uint32_t requested_movie,
				       std::vector<uint32_t> & matched_user_ids);

auto main( int argc, char **argv ) -> int {

  if( argc != 6 ) {
    std::cerr << "Usage: " << argv[0] << " <filename> <random_seed> <train_set_percent> <k> <k_threshold>" << std::endl;
    exit(1);
  }

  std::string s{ argv[1] };
  uint32_t random_seed = std::stoi( argv[2] );
  float train_set_percent = std::stof( argv[3] );
  Gk = std::stoi( argv[4] );
  Gt = std::stof( argv[5] );


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
  std::map<uint16_t,std::map<uint32_t,uint8_t>> train_movie_users;
  std::map<uint32_t,std::map<uint16_t,uint8_t>> test_users;
  std::map<uint16_t,std::vector<uint8_t>> train_movies;
  std::map<uint16_t,float> train_movie_averages;
  
  for( auto & u : train_user_ids ) {  // vector of user ids
    train_users[u] = users[u];
    for( auto & m : users[u] ) {      // user -> map[movie] = rating
      train_movies[m.first].push_back(m.second);
      train_movie_users[m.first][u] = m.second;
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

  uint32_t count = 0;
  uint32_t cold_start_count = 0;
  
  for( auto & tu : test_users ) {
    ++count;
    if( count % 100 == 0 ) {
      std::cout << count << std::endl;
    }
    auto movies_rated = tu.second.size();
    std::uniform_int_distribution<uint32_t> dist(0, movies_rated - 1);
    auto random_offset = dist(random_generator);
    auto item = tu.second.begin();
    std::advance( item, random_offset );
    //std::cout << "movie " << (*item).first << " : " << static_cast<uint32_t>((*item).second) << std::endl;
    auto test_rating = static_cast<float>((*item).second);
    auto test_movie = (*item).first;
    auto avg_rating = train_movie_averages[test_movie];

    std::vector<uint32_t> matched_users = {};
    
    //if( find_top_knn_users_cosine_record_debug( train_movie_users, train_users, test_users,
    //						tu.first, test_movie,
    //						matched_users ) ) {
    
    if( find_top_knn_users_cosine_record_debug( train_movie_users, train_users, test_users,
						tu.first, test_movie,
						matched_users ) ) {
      // Get matched_users ratings.
      //std::cout << "matched user = " << matched_users[0] << " ";
      float ratings_sum = 0;
      float ratings_count = 0;
      for( auto & mu : matched_users ) {
	ratings_sum += static_cast<float>(train_users[mu][test_movie]);
	ratings_count += 1.0f;
      }
      if( ratings_count <= 0 ) {
	auto test_rating = static_cast<float>((*item).second);
	auto test_movie = (*item).first;
	auto avg_rating = train_movie_averages[test_movie];
	rmse_N += ((test_rating - avg_rating)*(test_rating - avg_rating));
	++rmse_D;
      }	else {
	float rating = ratings_sum / ratings_count;  
	auto test_rating = static_cast<float>((*item).second);
	std::cout << "rating = " << rating << " actual = " << test_rating << std::endl;
	rmse_N += ((test_rating - rating)*(test_rating - rating));
	++rmse_D;
	if( rmse_D > 0 ) {
		std::cout << "RMSE = " << sqrt( rmse_N / rmse_D ) <<
		  "  cold start count = " << cold_start_count << " / " << count << std::endl;
	}
      }
    } else { // Not one else rated!  Grab average as best recourse.      
      auto test_rating = static_cast<float>((*item).second);
      auto test_movie = (*item).first;
      auto avg_rating = train_movie_averages[test_movie];
      rmse_N += ((test_rating - avg_rating)*(test_rating - avg_rating));
      ++rmse_D;
      ++cold_start_count;
      //if( rmse_D > 0 ) {
      // std::cout << "RMSE = " << sqrt( rmse_N / rmse_D ) <<
      //	  "  cold start count = " << cold_start_count << " / " << count << std::endl;
      //}
    }
  }

  if( rmse_D > 0 ) {
    std::cout << "RMSE = " << sqrt( rmse_N / rmse_D ) << "  k = " << Gk << " cold start count = " << cold_start_count << " / " << count << std::endl;
  }
  
  return(0);

}

bool find_top_knn_users_pearson( std::map<uint16_t,std::map<uint32_t,uint8_t>> & train_movie_users,
				 std::map<uint32_t,std::map<uint16_t,uint8_t>> & train_user_movies,
				 std::map<uint32_t,std::map<uint16_t,uint8_t>> & test_users,
				 uint32_t user_id,
				 uint32_t requested_movie,
				 std::vector<uint32_t> & matched_user_ids) {
  // Get all training users that rated the requested movie.
  std::vector<uint32_t> users_who_watched = {};
  for( auto & train_mu : train_movie_users[requested_movie] ) {
    users_who_watched.push_back( train_mu.first );
  }
  // Simplify lookup for the test user.  Just need to create movie/ratings hash for it.
  std::map<uint16_t,uint8_t> test_user_movie = {};
  for( auto & tum : test_users[user_id] ) {
    if( tum.first != requested_movie ) {
      test_user_movie[tum.first] = tum.second;
    }
  }
  auto tum_rated = test_user_movie.size();
  if( tum_rated == 0 ) {
    return( false );
  }
  std::vector<nn> knn = {};
  for( auto & wuser_id : users_who_watched ) {
    uint32_t matched_count = 0;
    std::vector<uint32_t> a = {};
    std::vector<uint32_t> b = {};
    for( auto & test_movie : test_user_movie ) {
      auto train_movie_rating = train_user_movies[wuser_id].find( test_movie.first );
      if( train_movie_rating != train_user_movies[wuser_id].end() ) {
	++matched_count;
	a.push_back( test_movie.second );
	b.push_back( (*train_movie_rating).second );
      }
    }
    if( matched_count > 0 ) {
      uint32_t a_sum = 0;
      uint32_t b_sum = 0;
      for( auto & n : a ) {
	a_sum += n;
      }
      for( auto & n : b ) {
	b_sum += n;
      }
      float a_avg = static_cast<float>(a_sum) / static_cast<float>( a.size() );
      float b_avg = static_cast<float>(b_sum) / static_cast<float>( b.size() );
      
      float AB = 0;
      float A2 = 0;
      float B2 = 0;
      
      for( uint32_t i = 0; i < a.size(); ++i ) {
	float adiff = a_avg - static_cast<float>(a[i]);
	float bdiff = b_avg - static_cast<float>(b[i]);
	AB += (adiff * bdiff);
	A2 += (adiff * adiff);
	B2 += (bdiff * bdiff);	
      }
      float D = sqrt(A2) * sqrt(B2);
      if( D != 0 ) {
	auto pearson_dist = static_cast<float>(AB) / D;
	add_to_knn_pearson( wuser_id, matched_count, pearson_dist, knn );
	//std::cout << pearson_dist << std::endl;
      }
    }
  }

  for( auto & n : knn ) {  
    matched_user_ids.push_back( n.user_id );
  }
  
  //std::cout << users_above_threshold << std::endl;

  return( true );
}

bool find_top_knn_users_pearson_a( std::map<uint16_t,std::map<uint32_t,uint8_t>> & train_movie_users,
				 std::map<uint32_t,std::map<uint16_t,uint8_t>> & train_user_movies,
				 std::map<uint32_t,std::map<uint16_t,uint8_t>> & test_users,
				 uint32_t user_id,
				 uint32_t requested_movie ) {
  // Get all training users that rated the requested movie.
  std::vector<uint32_t> users_who_watched = {};
  for( auto & train_mu : train_movie_users[requested_movie] ) {
    users_who_watched.push_back( train_mu.first );
  }
  // Simplify lookup for the test user.  Just need to create movie/ratings hash for it.
  std::map<uint16_t,uint8_t> test_user_movie = {};
  for( auto & tum : test_users[user_id] ) {
    if( tum.first != requested_movie ) {
      test_user_movie[tum.first] = tum.second;
    }
  }
  auto tum_rated = test_user_movie.size();
  if( tum_rated == 0 ) {
    return( false );
  }
  uint32_t users_above_threshold = 0;
  for( auto & wuser_id : users_who_watched ) {
    uint32_t matched_count = 0;
    std::vector<uint32_t> a = {};
    std::vector<uint32_t> b = {};
    for( auto & test_movie : test_user_movie ) {
      auto train_movie_rating = train_user_movies[wuser_id].find( test_movie.first );
      if( train_movie_rating != train_user_movies[wuser_id].end() ) {
	++matched_count;
	a.push_back( test_movie.second );
	b.push_back( (*train_movie_rating).second );
      }
    }
    if( matched_count > 0 ) {
      uint32_t a_sum = 0;
      uint32_t b_sum = 0;
      for( auto & n : a ) {
	a_sum += n;
      }
      for( auto & n : b ) {
	b_sum += n;
      }
      float a_avg = static_cast<float>(a_sum) / static_cast<float>( a.size() );
      float b_avg = static_cast<float>(b_sum) / static_cast<float>( b.size() );
      
      float AB = 0;
      float A2 = 0;
      float B2 = 0;
      
      for( uint32_t i = 0; i < a.size(); ++i ) {
	float adiff = a_avg - static_cast<float>(a[i]);
	float bdiff = b_avg - static_cast<float>(b[i]);
	AB += (adiff * bdiff);
	A2 += (adiff * adiff);
	B2 += (bdiff * bdiff);	
      }
      float D = sqrt(A2) * sqrt(B2);
      if( D != 0 ) {
	auto pearson_dist = static_cast<float>(AB) / D;
	//std::cout << pearson_dist << std::endl;
      }
    }
  }

  //std::cout << users_above_threshold << std::endl;

  return( true );
}

bool find_top_knn_users_cosine( std::map<uint16_t,std::map<uint32_t,uint8_t>> & train_movie_users,
				std::map<uint32_t,std::map<uint16_t,uint8_t>> & train_user_movies,
				std::map<uint32_t,std::map<uint16_t,uint8_t>> & test_users,
				uint32_t user_id,
				uint32_t requested_movie ) {
  // Get all training users that rated the requested movie.
  std::vector<uint32_t> users_who_watched = {};
  for( auto & train_mu : train_movie_users[requested_movie] ) {
    users_who_watched.push_back( train_mu.first );
  }
  // Simplify lookup for the test user.  Just need to create movie/ratings hash for it.
  std::map<uint16_t,uint8_t> test_user_movie = {};
  for( auto & tum : test_users[user_id] ) {
    if( tum.first != requested_movie ) {
      test_user_movie[tum.first] = tum.second;
    }
  }
  auto tum_rated = test_user_movie.size();
  if( tum_rated == 0 ) {
    return( false );
  }
  uint32_t users_above_threshold = 0;
  for( auto & wuser_id : users_who_watched ) {
    uint32_t AB = 0;
    uint32_t A2 = 0;
    uint32_t B2 = 0;
    uint32_t matched_count = 0;
    for( auto & test_movie : test_user_movie ) {
      auto train_movie_rating = train_user_movies[wuser_id].find( test_movie.first );
      if( train_movie_rating != train_user_movies[wuser_id].end() ) {
	++matched_count;
	uint32_t a = test_movie.second;
	uint32_t b = (*train_movie_rating).second;
	AB += a*b;
	A2 += a*a;
	B2 += b*b;
	//std::cout << "a = " << a << " b = " << b << std::endl;
      }
    }
    if( matched_count > 1 ) {
      ++users_above_threshold;
      float D = sqrt(A2) * sqrt(B2);
      if( D != 0 ) {
	auto cos_dist = static_cast<float>(AB) / D;
	//std::cout << cos_dist << std::endl;
      }
    }
  }

  //std::cout << users_above_threshold << std::endl;

  return( true );
}

bool find_top_knn_users_cosine_record_debug( std::map<uint16_t,std::map<uint32_t,uint8_t>> & train_movie_users,
					     std::map<uint32_t,std::map<uint16_t,uint8_t>> & train_user_movies,
					     std::map<uint32_t,std::map<uint16_t,uint8_t>> & test_users,
					     uint32_t user_id,
					     uint32_t requested_movie,
					     std::vector<uint32_t> & matched_user_ids) {
  // Get all training users that rated the requested movie.
  std::vector<uint32_t> users_who_watched = {};
  for( auto & train_mu : train_movie_users[requested_movie] ) {
    users_who_watched.push_back( train_mu.first );
  }
  // Simplify lookup for the test user.  Just need to create movie/ratings hash for it.
  std::map<uint16_t,uint8_t> test_user_movie = {};
  for( auto & tum : test_users[user_id] ) {
    if( tum.first != requested_movie ) {
      test_user_movie[tum.first] = tum.second;
    }
  }
  uint32_t target_test_rating = test_users[user_id][requested_movie];

  auto tum_rated = test_user_movie.size();
  if( tum_rated == 0 ) {
    return( false );
  }
  //  uint32_t users_above_threshold = 0;
  //
  //bool highest_user_set = false;
  //float highest_user_match_rating = 0;
  //uint32_t highest_user_matched_rated_movies;
  //uint32_t highest_user_id;

  std::vector<nn> knn = {};
  
  for( auto & wuser_id : users_who_watched ) {
    uint32_t AB = 0;
    uint32_t A2 = 0;
    uint32_t B2 = 0;
    uint32_t matched_count = 0;
    //std::cout << "train user = " << wuser_id << ":" << std::endl;
    for( auto & test_movie : test_user_movie ) {
      auto train_movie_rating = train_user_movies[wuser_id].find( test_movie.first );
      if( train_movie_rating != train_user_movies[wuser_id].end() ) {
	++matched_count;
	uint32_t a = test_movie.second;
	uint32_t b = (*train_movie_rating).second;
	AB += a*b;
	A2 += a*a;
	B2 += b*b;
	//std::cout << "     a = " << a << " b = " << b << std::endl;
      }
    }
    float D = sqrt(A2) * sqrt(B2);
    if( D != 0 ) {
      float cos_dist = static_cast<float>(AB) / D;
      add_to_knn( wuser_id, matched_count, cos_dist, knn );
    }
//    if( matched_count >= 100 ) {
//      //std::cout << "matched_count = " << matched_count << " for user " << wuser_id << std::endl;
//      float cos_dist = 0.0f;
//      ++users_above_threshold;
//      float D = sqrt(A2) * sqrt(B2);
//      if( D != 0 ) {
//	cos_dist = static_cast<float>(AB) / D;
//	//std::cout << cos_dist << std::endl;
//      }
//      if( !highest_user_set ) {
//	highest_user_set = true;
//	highest_user_match_rating = cos_dist;
//        highest_user_matched_rated_movies = matched_count;
//	highest_user_id = wuser_id;
//      } else {
//	if( matched_count >= highest_user_matched_rated_movies ) {
//	  highest_user_match_rating = cos_dist;
//	  highest_user_matched_rated_movies = matched_count;
//	  highest_user_id = wuser_id;
//	} else if( (matched_count == highest_user_matched_rated_movies) && (cos_dist > highest_user_match_rating) ) {
//	  highest_user_match_rating = cos_dist;
//	  highest_user_matched_rated_movies = matched_count;
//	  highest_user_id = wuser_id;
//	} else if( cos_dist > highest_user_match_rating ) {
//	  highest_user_match_rating = cos_dist;
//	  highest_user_matched_rated_movies = matched_count;
//	  highest_user_id = wuser_id;
//	}
//      }
//    } else {
//      return( false );
//      //if( highest_user_match_rating < 1.01 ) {
//      //	return( false );
//      //}
//    }
    
  }

  //std::cout << "h match rating = " << highest_user_match_rating << " count = " << highest_user_matched_rated_movies << std::endl;

  for( auto & n : knn ) {  
    matched_user_ids.push_back( n.user_id );
  }
  
  //std::cout << users_above_threshold << std::endl;

  return( true );
}

bool find_top_knn_users_cosine_record( std::map<uint16_t,std::map<uint32_t,uint8_t>> & train_movie_users,
				       std::map<uint32_t,std::map<uint16_t,uint8_t>> & train_user_movies,
				       std::map<uint32_t,std::map<uint16_t,uint8_t>> & test_users,
				       uint32_t user_id,
				       uint32_t requested_movie,
				       std::vector<uint32_t> & matched_user_ids) {
  // Get all training users that rated the requested movie.
  std::vector<uint32_t> users_who_watched = {};
  for( auto & train_mu : train_movie_users[requested_movie] ) {
    users_who_watched.push_back( train_mu.first );
  }
  // Simplify lookup for the test user.  Just need to create movie/ratings hash for it.
  std::map<uint16_t,uint8_t> test_user_movie = {};
  for( auto & tum : test_users[user_id] ) {
    if( tum.first != requested_movie ) {
      test_user_movie[tum.first] = tum.second;
    }
  }
  auto tum_rated = test_user_movie.size();
  if( tum_rated == 0 ) {
    return( false );
  }
  uint32_t users_above_threshold = 0;

  bool highest_user_set = false;
  float highest_user_match_rating = 0;
  uint32_t highest_user_matched_rated_movies;
  uint32_t highest_user_id;
  for( auto & wuser_id : users_who_watched ) {
    uint32_t AB = 0;
    uint32_t A2 = 0;
    uint32_t B2 = 0;
    uint32_t matched_count = 0;
    for( auto & test_movie : test_user_movie ) {
      auto train_movie_rating = train_user_movies[wuser_id].find( test_movie.first );
      if( train_movie_rating != train_user_movies[wuser_id].end() ) {
	++matched_count;
	uint32_t a = test_movie.second;
	uint32_t b = (*train_movie_rating).second;
	AB += a*b;
	A2 += a*a;
	B2 += b*b;
	//std::cout << "a = " << a << " b = " << b << std::endl;
      }
    }
    if( matched_count >= 1 ) {
      //std::cout << "matched_count = " << matched_count << " for user " << wuser_id << std::endl;
      float cos_dist = 0.0f;
      ++users_above_threshold;
      float D = sqrt(A2) * sqrt(B2);
      if( D != 0 ) {
	cos_dist = static_cast<float>(AB) / D;
	//std::cout << cos_dist << std::endl;
      }
      if( !highest_user_set ) {
	highest_user_set = true;
	highest_user_match_rating = cos_dist;
        highest_user_matched_rated_movies = matched_count;
	highest_user_id = wuser_id;
      } else {
	if( matched_count >= highest_user_matched_rated_movies ) {
	  highest_user_match_rating = cos_dist;
	  highest_user_matched_rated_movies = matched_count;
	  highest_user_id = wuser_id;
	} else if( (matched_count == highest_user_matched_rated_movies) && (cos_dist > highest_user_match_rating) ) {
	  highest_user_match_rating = cos_dist;
	  highest_user_matched_rated_movies = matched_count;
	  highest_user_id = wuser_id;
	} else if( cos_dist > highest_user_match_rating ) {
	  highest_user_match_rating = cos_dist;
	  highest_user_matched_rated_movies = matched_count;
	  highest_user_id = wuser_id;
	}
      }
    } else {
      return( false );
      //if( highest_user_match_rating < 1.01 ) {
      //	return( false );
      //}
    }
    
  }

  //std::cout << "h match rating = " << highest_user_match_rating << " count = " << highest_user_matched_rated_movies << std::endl;
  
  matched_user_ids.push_back( highest_user_id );
  
  //std::cout << users_above_threshold << std::endl;

  return( true );
}

void add_to_knn( uint32_t wuser_id, uint32_t matched_count, float cos_dist, std::vector<nn> & knn ) {

  uint32_t k = Gk;

  if( knn.size() < k ) {
    knn.push_back( {wuser_id, matched_count, cos_dist} );
  } else {
    // Ignore matched count for now!
    float lowest_cos_dist = knn[0].cos_dist;
    int32_t lowest_index = -1;
    uint32_t index = 0;
    for( auto & i : knn ) {
      if( (lowest_cos_dist < i.cos_dist)  ) {
	lowest_cos_dist = i.cos_dist;
	lowest_index = index;
      }
      ++index;
    }
    if( lowest_index != -1 ) {
      knn[lowest_index] = {wuser_id, matched_count, cos_dist};
    }
  }
  
}

void add_to_knn_b( uint32_t wuser_id, uint32_t matched_count, float cos_dist, std::vector<nn> & knn ) {

  uint32_t k = Gk;

  if( knn.size() < k ) {
    knn.push_back( {wuser_id, matched_count, cos_dist} );
  } else { // Get lowest matched count.
    uint32_t lowest_matched_count = knn[0].matched_count;
    for( auto & i : knn ) {
      if( lowest_matched_count < i.matched_count ) {
	lowest_matched_count = i.matched_count;
      }
    }
    if( matched_count < lowest_matched_count ) {
      return;
    }
    // Of the lowest matched counts, get the lowest cos_dist.
    float lowest_cos_dist = knn[0].cos_dist;
    int32_t lowest_index = -1;
    uint32_t index = 0;
    for( auto & i : knn ) {
      if( (lowest_cos_dist < i.cos_dist) && (lowest_matched_count == i.matched_count) ) {
	lowest_cos_dist = i.cos_dist;
	lowest_index = index;
      }
      ++index;
    }
    if( lowest_index != -1 ) {
      knn[lowest_index] = {wuser_id, matched_count, cos_dist};
    }
  }
  
}

void add_to_knn_threshold( uint32_t wuser_id, uint32_t matched_count, float cos_dist, std::vector<nn> & knn ) {

  float t = Gt;

  if( cos_dist > t ) {
    knn.push_back( {wuser_id, matched_count, cos_dist} );
  }
  
}

void add_to_knn_pearson( uint32_t wuser_id, uint32_t matched_count, float dist, std::vector<nn> & knn ) {

  uint32_t k = Gk;

  if( knn.size() < k ) {
    knn.push_back( {wuser_id, matched_count, dist} );
  } else {
    // Ignore matched count for now!
    float lowest_dist = knn[0].cos_dist;
    int32_t lowest_index = -1;
    uint32_t index = 0;
    for( auto & i : knn ) {
      if( (lowest_dist < i.cos_dist)  ) {
	lowest_dist = i.cos_dist;
	lowest_index = index;
      }
      ++index;
    }
    if( lowest_index != -1 ) {
      knn[lowest_index] = {wuser_id, matched_count, dist};
    }
  }
  
}
