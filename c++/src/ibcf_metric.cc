#include <algorithm>
#include <cmath>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <set>
#include <string>
#include <vector>

#include "fold_generation.h"
#include "ibcf_metric.h"
#include "load_data.h"

auto main( int argc, char **argv ) -> int {

  if( argc != 7 ) {
    std::cerr << "Usage: " << argv[0] << " <filename> <random_seed> <folds> <train_test_percent> <k> <similarity_method>"
	      << std::endl;
    exit(1);
  }

  std::string s{ argv[1] };
  uint32_t    random_seed       = std::stoi( argv[2] );
  uint32_t    folds             = std::stoi( argv[3] );
  float       train_set_percent = std::stof( argv[4] );
  uint32_t    k                 = std::stoi( argv[5] );
  std::string method_arg{ argv[6] };
  bool pearson = false;
  bool cosine  = false;
  if( method_arg == "p" ) {
    pearson = true;
  } else if( method_arg == "c" ) {
    cosine = true;
  } else {
    std::cerr << "Method argument must be 'p' or 'c'." << std::endl;
  }
  
  //float       t                 = std::stof( argv[5] );  // Threshold way of knn, elided for now.

  if( folds < 1 ) {
    std::cout << "Fold count must be >= 1." << std::endl;
    exit( 1 );
  }
  
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
  
  uint32_t total_folds = folds;

  std::vector<float> RMSE( total_folds, 0.0f );

  for( uint32_t fold = 0; fold < total_folds; ++fold ) {
    
    std::vector<uint32_t> train_user_ids = {};
    std::vector<uint32_t> test_user_ids  = {};
    
    if( total_folds == 1 ) { // Going into percentage mode for test generation.
      extract_fold_user_ids_percentage_method( random_user_ids, train_set_percent, train_user_ids, test_user_ids );
    } else {    
      extract_fold_user_ids( random_user_ids, fold, total_folds, train_user_ids, test_user_ids );
    }

    // Build train and test map.

    // For IBCF need to examine each training user's movies and accumulate similarity attributes
    // for that given movie pair.  This entire matrix can be stored in memory by using the smaller
    // movie id as the first index and the larger one as the second.  This will produce a triangular
    // matrix.

    // Triangular matrix will be a map [first_movie][second_movie] = struct{ std::vector<float> A, std::vector<float> B

    std::map<uint32_t,std::map<uint16_t,uint8_t>> train_users;
    std::map<uint16_t,std::map<uint32_t,uint8_t>> train_movie_users;
    std::map<uint32_t,std::map<uint16_t,uint8_t>> test_users;
    std::map<uint16_t,std::vector<uint8_t>> train_movies;
    std::map<uint16_t,float> train_movie_averages;
    
    std::map<uint32_t,std::map<uint16_t,float>> Norm_train_users;
    std::map<uint32_t,std::map<uint16_t,float>> Norm_test_users;
    
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
  
    // Create normalized versions of the above.

    normalize_from( train_users, Norm_train_users );
    normalize_from( test_users, Norm_test_users );

    // Build similarity item-item look-ups.

    std::map<uint32_t,std::map<uint32_t,similarity_info>> ibcf_matrix;
    
    build_item_item_matrix( Norm_train_users, movies, ibcf_matrix, cosine, pearson );

    // RMSE start!

    float rmse_N = 0.0f;
    float rmse_D = 0.0f;
    
    uint32_t count = 0;
    uint32_t cold_start_count = 0;
  
    for( auto & tu : test_users ) {
      ++count;
      //if( count % 100 == 0 ) {
      //std::cout << count << std::endl;
      //}
      auto movies_rated = tu.second.size();
      std::uniform_int_distribution<uint32_t> dist(0, movies_rated - 1);
      auto random_offset = dist(random_generator);
      auto item = tu.second.begin();
      std::advance( item, random_offset );
      auto test_rating = static_cast<float>((*item).second);
      auto test_movie = (*item).first;
      auto avg_rating = train_movie_averages[test_movie];
      
      std::vector<uint16_t> matched_movies = {};
      std::vector<uint32_t> matched_users = {};
      
      //    if( find_top_knn_users_cosine( k, train_movie_users, train_users, test_users,
      //				   tu.first, test_movie,
      //				   matched_users ) ) {
      bool warm_reading = false;
      if( cosine ) {
	//warm_reading = find_top_knn_users_cosine_normalized( k, train_movie_users, Norm_train_users, Norm_test_users,
	//						     tu.first, test_movie,
	//						     matched_users );

	// Got similarity matrix.
	// Got test user and ratings.
	// Don't need anything more -- no need to separate pearson and cosine, at least as much.
	
	//	warm_reading = find_top_knn_movies_cosine_normalized( k, train_movie_users, Norm_train_users, Norm_test_users,
	//						      tu.first, test_movie,
	//						      matched_movies );
      } else if( pearson ) {
	warm_reading = find_top_knn_users_pearson_normalized( k, train_movie_users, Norm_train_users, Norm_test_users,
							      tu.first, test_movie,
							      matched_users );
      }
      if( warm_reading ) {
	// Get matched_users ratings.
	//std::cout << "matched user = " << matched_users[0] << " ";
	float ratings_sum = 0;
	float ratings_count = 0;
	float test_user_bias = compute_user_bias( tu.first, test_users, train_movie_averages, true, test_movie );
	for( auto & mu : matched_users ) {
	  ratings_sum += (static_cast<float>(train_users[mu][test_movie]) - compute_user_bias( mu, train_users, train_movie_averages, false, 0 ));
	  ratings_count += 1.0f;
	}
	if( ratings_count <= 0 ) {
	  auto test_rating = static_cast<float>((*item).second);
	  auto test_movie = (*item).first;
	  auto avg_rating = train_movie_averages[test_movie];
	  rmse_N += ((test_rating - avg_rating)*(test_rating - avg_rating));
	  ++rmse_D;
	} else {
	  float rating = ratings_sum / ratings_count + test_user_bias;
	  auto test_rating = static_cast<float>((*item).second);

	  // Blend if below k.
	  auto test_movie = (*item).first;
	  auto avg_rating = train_movie_averages[test_movie];
	  float frac_w = static_cast<float>(k - matched_users.size()) / static_cast<float>( k );
	  float knn_w = 1.0 - frac_w;

	  rating = frac_w * avg_rating + knn_w * rating;
	  
	  rmse_N += ((test_rating - rating)*(test_rating - rating));
	  ++rmse_D;
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
      RMSE[fold] = sqrt( rmse_N / rmse_D );
      //std::cout << "RMSE = " << RMSE[fold] << "  k = " << k << " fold = " << fold << " cold start count = " << cold_start_count << " / " << count << std::endl;
    }
  }

  float RMSE_sum = 0.0;
  for( auto & rmse : RMSE ) {
    RMSE_sum += rmse;
  }
  
  std::cout << "Final RMSE = " << (RMSE_sum/static_cast<float>(RMSE.size())) << "  k = " << k << std::endl;

  
  return(0);

}

bool find_top_knn_users_pearson( uint32_t k,
				 std::map<uint16_t,std::map<uint32_t,uint8_t>> & train_movie_users,
				 std::map<uint32_t,std::map<uint16_t,uint8_t>> & train_user_movies,
				 std::map<uint32_t,std::map<uint16_t,uint8_t>> & test_users,
				 uint32_t user_id,
				 uint32_t requested_movie,
				 std::vector<uint32_t> & matched_user_ids ) {
  
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
	add_to_knn( k, wuser_id, matched_count, pearson_dist, knn );
      }
    }
  }

  for( auto & n : knn ) {  
    matched_user_ids.push_back( n.user_id );
  }
  
  return( true );
}

bool find_top_knn_users_pearson_normalized
( uint32_t k,
  std::map<uint16_t,std::map<uint32_t,uint8_t>> & train_movie_users,
  std::map<uint32_t,std::map<uint16_t,float>> & train_user_movies,
  std::map<uint32_t,std::map<uint16_t,float>> & test_users,
  uint32_t user_id,
  uint32_t requested_movie,
  std::vector<uint32_t> & matched_user_ids ) {
  
  // Get all training users that rated the requested movie.
  std::vector<uint32_t> users_who_watched = {};
  for( auto & train_mu : train_movie_users[requested_movie] ) {
    users_who_watched.push_back( train_mu.first );
  }
  // Simplify lookup for the test user.  Just need to create movie/ratings hash for it.
  std::map<uint16_t,float> test_user_movie = {};
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
	add_to_knn( k, wuser_id, matched_count, pearson_dist, knn );
      }
    }
  }

  for( auto & n : knn ) {  
    matched_user_ids.push_back( n.user_id );
  }
  
  return( true );
}


bool find_top_knn_users_cosine( uint32_t k,
				std::map<uint16_t,std::map<uint32_t,uint8_t>> & train_movie_users,
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
      add_to_knn( k, wuser_id, matched_count, cos_dist, knn );
    }
  }

  for( auto & n : knn ) {  
    matched_user_ids.push_back( n.user_id );
  }
  
  return( true );
}

bool find_top_knn_users_cosine_normalized
( uint32_t k,
  std::map<uint16_t,std::map<uint32_t,uint8_t>> & train_movie_users,
  std::map<uint32_t,std::map<uint16_t,float>> & train_user_movies,
  std::map<uint32_t,std::map<uint16_t,float>> & test_users,
  uint32_t user_id,
  uint32_t requested_movie,
  std::vector<uint32_t> & matched_user_ids) {
  
  // Get all training users that rated the requested movie.
  std::vector<uint32_t> users_who_watched = {};
  for( auto & train_mu : train_movie_users[requested_movie] ) {
    users_who_watched.push_back( train_mu.first );
  }
  // Simplify lookup for the test user.  Just need to create movie/ratings hash for it.
  std::map<uint16_t,float> test_user_movie = {};
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
    float AB = 0;
    float A2 = 0;
    float B2 = 0;
    uint32_t matched_count = 0;
    //std::cout << "train user = " << wuser_id << ":" << std::endl;
    for( auto & test_movie : test_user_movie ) {
      auto train_movie_rating = train_user_movies[wuser_id].find( test_movie.first );
      if( train_movie_rating != train_user_movies[wuser_id].end() ) {
	++matched_count;
	//uint32_t a = trunc(test_movie.second);
	//uint32_t b = trunc((*train_movie_rating).second);
	//	uint32_t a = round(test_movie.second);
	//	uint32_t b = round((*train_movie_rating).second);
	float    a = test_movie.second;
	float    b = (*train_movie_rating).second;
	AB += a*b;
	A2 += a*a;
	B2 += b*b;
	//std::cout << "     a = " << a << " b = " << b << std::endl;
      }
    }
    float D = sqrt(A2) * sqrt(B2);
    if( D != 0 ) {
      float cos_dist = static_cast<float>(AB) / D;
      add_to_knn( k, wuser_id, matched_count, cos_dist, knn );
    }
  }

  for( auto & n : knn ) {  
    matched_user_ids.push_back( n.user_id );
  }
  
  return( true );
}


void add_to_knn( uint32_t k, uint32_t wuser_id, uint32_t matched_count,
		 float dist, std::vector<nn> & knn ) {

  if( knn.size() < k ) {
    knn.push_back( {wuser_id, matched_count, dist} );
  } else {
    // Ignore matched count for now!
    float lowest_dist = knn[0].dist;
    int32_t lowest_index = -1;
    uint32_t index = 0;
    for( auto & i : knn ) {
      if( (lowest_dist < i.dist)  ) {
	lowest_dist = i.dist;
	lowest_index = index;
      }
      ++index;
    }
    if( lowest_index != -1 ) {
      knn[lowest_index] = {wuser_id, matched_count, dist};
    }
  }
  
}

void normalize_from( std::map<uint32_t,std::map<uint16_t,uint8_t>> & u_m_r,
		     std::map<uint32_t,std::map<uint16_t,float>> & u_m_r_OUT ) {
  // Copy from ints to floats
  for( auto & u : u_m_r ) {
    uint32_t movie_rating_sum = 0;
    uint32_t movie_rating_count = 0;
    u_m_r_OUT[u.first] = {};
    for( auto & m : u_m_r[u.first] ) {
      movie_rating_sum += m.second;
      ++movie_rating_count;
    }
    float average = static_cast<float>( movie_rating_sum ) / static_cast<float>( movie_rating_count );
    for( auto & m : u_m_r[u.first] ) {
      u_m_r_OUT[u.first][m.first] = static_cast<float>( m.second ) - average;
    }
  }
}

float compute_user_bias( uint32_t user_id,
			 std::map<uint32_t,std::map<uint16_t,uint8_t>> & train_users,
			 std::map<uint16_t,float> train_movie_averages,
			 bool skip_test_movie,
			 uint32_t test_movie ) {

  float bias = 0.0f;
  float N = 0.0f;

  for( auto & m_r : train_users[user_id] ) {
    if( skip_test_movie ) {
      if( (m_r.first == test_movie) && (skip_test_movie) ) {
	continue;
      }
    }
    float avg = train_movie_averages[m_r.first];
    float user_rating = static_cast<float>(m_r.second);
    bias += (user_rating - avg);
    N += 1.0f;
  }

  return( bias / N );
  
}

void build_item_item_matrix( std::map<uint32_t,std::map<uint16_t,float>> & Norm_train_users,
			     std::map<uint16_t,std::map<uint32_t,uint8_t>> & movies,
			     std::map<uint32_t,std::map<uint32_t,similarity_info>> & ibcf_matrix,
			     bool cosine,
			     bool pearson ) {
  
  // Initialize all slots to be used.

  std::vector<uint16_t> movie_ids = {};

  for( auto & m_u_r : movies ) {
    movie_ids.push_back( m_u_r.first );
  }

  std::sort( movie_ids.begin(), movie_ids.end() );
  
  for( uint32_t first = 0; first < movie_ids.size() - 1; ++first ) {
    for( uint32_t second = first + 1; second < movie_ids.size(); ++second ) {
      ibcf_matrix[first][second] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
    }
  }

  // Slots initialized, add-in metrics.

  std::cerr << "Pass 1... " << std::endl;
  
  for( auto & u_m_r : Norm_train_users ) {    
    // Wish I could mess with iterators, but I need to get project done.
    auto & m_r = u_m_r.second;
    std::vector<uint16_t> Nmovies = {};
    for( auto & m : u_m_r.second ) {
      Nmovies.push_back( m.first );
    }
    std::sort( Nmovies.begin(), Nmovies.end() );
    for( uint32_t first = 0; first < Nmovies.size() - 1; ++first ) {
      auto & i_m_f = ibcf_matrix[first];
      for( uint32_t second = first + 1; second < Nmovies.size(); ++second ) {
	auto & i_m_f_s = i_m_f[second];
	if( pearson ) {
	  i_m_f_s.A_sum += m_r[movie_ids[first]];
	  i_m_f_s.B_sum += m_r[movie_ids[second]];
	  i_m_f_s.count += 1.0;
	} else {
	  float A = m_r[movie_ids[first]];
	  float B = m_r[movie_ids[second]];
	  i_m_f_s.numerator_AB += (A*B);
	  i_m_f_s.denominator_A2 += (A*A);
	  i_m_f_s.denominator_B2 += (B*B);
	}
      }
    }
  }

  std::cerr << "Pass 2 (Averages construction)... " << std::endl;
  // Compute the averages now.

  for( auto & i : ibcf_matrix ) {
    for( auto & ii : ibcf_matrix[i.first] ) {
      if( pearson ) {
	ii.second.A_average = ii.second.A_sum / ii.second.count;
	ii.second.B_average = ii.second.B_sum / ii.second.count;
      } else {
	float D = sqrt( ii.second.denominator_A2 ) * sqrt( ii.second.denominator_B2 );
	ii.second.similarity = ii.second.numerator_AB / D;
      }
    }
  }

  if( cosine ) {
    return;
  }
  
  std::cerr << "Pass 3... " << std::endl;
  
  for( auto & u_m_r : Norm_train_users ) {    
    // Wish I could mess with iterators, but I need to get project done.
    auto & m_r = u_m_r.second;
    std::vector<uint16_t> Nmovies = {};
    for( auto & m : u_m_r.second ) {
      Nmovies.push_back( m.first );
    }
    std::sort( Nmovies.begin(), Nmovies.end() );
    for( uint32_t first = 0; first < Nmovies.size() - 1; ++first ) {
      auto & i_m_f = ibcf_matrix[first];
      for( uint32_t second = first + 1; second < Nmovies.size(); ++second ) {
	auto & i_m_f_s = i_m_f[second];
	float adiff = i_m_f_s.A_average - m_r[movie_ids[first]];
	float bdiff = i_m_f_s.B_average - m_r[movie_ids[second]];
	i_m_f_s.numerator_AB += (adiff * bdiff);
	i_m_f_s.denominator_A2 += (adiff * adiff);
	i_m_f_s.denominator_B2 += (bdiff * bdiff);
      }
    }
  }

  std::cerr << "Pass 4 (Similarity construction)... " << std::endl;
  
  // Compute the averages now.

  for( auto & i : ibcf_matrix ) {
    for( auto & ii : ibcf_matrix[i.first] ) {
      float D = sqrt(ii.second.denominator_A2) * sqrt(ii.second.denominator_B2);
      ii.second.similarity = ii.second.numerator_AB / D;
    }
  }
  
}
