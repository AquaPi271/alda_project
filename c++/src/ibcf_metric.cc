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

const uint32_t dim = 18000;
static int32_t movie_combo_lookup[dim][dim] = {};
std::vector<similarity_info> similarity_vector = {};
const float default_no_similarity = -1000.0f;
  
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
      
      // Pick the random movie for this test user.
      
      auto movies_rated = tu.second.size();
      std::uniform_int_distribution<uint32_t> dist(0, movies_rated - 1);
      auto random_offset = dist(random_generator);
      auto item = tu.second.begin();
      std::advance( item, random_offset );
      auto test_rating = static_cast<float>((*item).second);
      auto test_movie = (*item).first;
      auto avg_rating = train_movie_averages[test_movie];
      
      std::vector<uint16_t> matched_movies = {};
      
      //    if( find_top_knn_users_cosine( k, train_movie_users, train_users, test_users,
      //				   tu.first, test_movie,
      //				   matched_users ) ) {
      bool warm_reading = false;
      if( cosine ) {
	warm_reading = find_top_knn_movies_cosine_normalized( k, Norm_test_users, tu.first, test_movie, matched_movies );
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
	warm_reading = find_top_knn_movies_pearson_normalized( k, Norm_test_users, tu.first, test_movie, matched_movies );
      }
      if( warm_reading ) {
	// Get matched movies  ratings.
	//std::cout << "matched user = " << matched_users[0] << " ";
	float ratings_sum = 0;
	float ratings_count = 0;
	//float test_user_bias = compute_user_bias( tu.first, test_users, train_movie_averages, true, test_movie );

	for( auto & mm : matched_movies ) {
	  // Get the rating that the test user had for this movie, do not need to discount bias because built-in for user.	  
	  ratings_sum += static_cast<float>(test_users[tu.first][mm]);
	  ratings_count += 1.0f;
	}
	
	if( ratings_count <= 0.0f ) {
	  auto test_rating = static_cast<float>((*item).second);
	  auto test_movie = (*item).first;
	  auto avg_rating = train_movie_averages[test_movie];
	  rmse_N += ((test_rating - avg_rating)*(test_rating - avg_rating));
	  ++rmse_D;
	} else {
	  float rating = ratings_sum / ratings_count;  // NOTE: bias not needed here because using own ratings.
	  auto test_rating = static_cast<float>((*item).second);

	  // Blend if below k.
	  auto test_movie = (*item).first;
	  auto avg_rating = train_movie_averages[test_movie];
	  float frac_w = static_cast<float>(k - matched_movies.size()) / static_cast<float>( k );
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
      std::cout << "RMSE = " << RMSE[fold] << "  k = " << k << " fold = " << fold << " cold start count = " << cold_start_count << " / " << count << std::endl;
    }
  }

  float RMSE_sum = 0.0;
  for( auto & rmse : RMSE ) {
    RMSE_sum += rmse;
  }
  
  std::cout << "Final RMSE = " << (RMSE_sum/static_cast<float>(RMSE.size())) << "  k = " << k << std::endl;

  
  return(0);

}

bool find_top_knn_movies_cosine_normalized
( uint32_t k,  
  std::map<uint32_t,std::map<uint16_t,float>> & test_users,
  uint32_t user_id,
  uint32_t requested_movie,
  std::vector<uint16_t> & matched_movies ) {

  std::vector<nn> knn = {};
  
  // Need to probe triangular matrix for similarity measurements, first horizontal, then vertical.

  uint32_t matched_count = 0;
  
  for( auto c = 0; c < dim; ++c ) {
    uint32_t lookup_index = movie_combo_lookup[requested_movie][c];
    if( lookup_index != -1 ) {      
      float dist = similarity_vector[lookup_index].similarity;
      add_to_knn( k, c, dist, knn );
    }
  }
  for( auto r = 0; r < dim; ++r ) {
    uint32_t lookup_index = movie_combo_lookup[r][requested_movie];
    if( lookup_index != -1 ) { 
      float dist = similarity_vector[lookup_index].similarity;     
      add_to_knn( k, r, dist, knn );
    }
  }

  for( auto & n : knn ) {  
    matched_movies.push_back( n.movie_id );
  }

  if( knn.size() == 1 ) {
    return(false);
  }
  
  return( true );
}

bool find_top_knn_movies_pearson_normalized
( uint32_t k,  
  std::map<uint32_t,std::map<uint16_t,float>> & test_users,
  uint32_t user_id,
  uint32_t requested_movie,
  std::vector<uint16_t> & matched_movies ) {

  std::vector<nn> knn = {};
  
  // Need to probe triangular matrix for similarity measurements, first horizontal, then vertical.

  uint32_t matched_count = 0;
  
  for( auto c = 0; c < dim; ++c ) {
    uint32_t lookup_index = movie_combo_lookup[requested_movie][c];
    if( lookup_index != -1 ) {      
      float dist = similarity_vector[lookup_index].similarity;
      add_to_knn( k, c, dist, knn );
    }
  }
  for( auto r = 0; r < dim; ++r ) {
    uint32_t lookup_index = movie_combo_lookup[r][requested_movie];
    if( lookup_index != -1 ) { 
      float dist = similarity_vector[lookup_index].similarity; 
      add_to_knn( k, r, dist, knn );
    }
  }

  for( auto & n : knn ) {
    matched_movies.push_back( n.movie_id );
  }

  if( knn.size() == 1 ) {
    return(false);
  }
  
  return( true );
}

void add_to_knn( uint32_t k, uint32_t wmovie_id, float dist, std::vector<nn> & knn ) {

  //  std::cout << dist << " " << " knn.size() = " << knn.size() << std::endl;
  
  if( knn.size() < k ) {
    knn.push_back( {wmovie_id, dist} );
  } else {
    float lowest_dist = knn[0].dist;
    uint32_t lowest_index = knn.size() + 1;
    uint32_t i = 0;
    for( i = 0; i < knn.size(); ++i ) {
      if( knn[i].dist <= lowest_dist ) {
	lowest_dist = knn[i].dist;
	lowest_index = i;
      }
    }

    if( lowest_index != knn.size() ) {
      if( dist > lowest_dist ) {
	knn[lowest_index].movie_id = wmovie_id;
	knn[lowest_index].dist = dist;
      }
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

void build_item_item_matrix( std::map<uint32_t,std::map<uint16_t,float>> & Norm_train_users,      // Normalized ratings u_m_r
			     std::map<uint16_t,std::map<uint32_t,uint8_t>> & movies,              // All movies in the dataset, might need for initialization
			     std::map<uint32_t,std::map<uint32_t,similarity_info>> & ibcf_matrix, // Matrix to use for similarity computations.
			     bool cosine,
			     bool pearson ) {
  
  // Initialize all slots to be used.

  //  std::vector<uint16_t> movie_ids = {};
//
//  for( auto & m_u_r : movies ) {
//    movie_ids.push_back( m_u_r.first );
//  }
//
//  std::sort( movie_ids.begin(), movie_ids.end() );
//  
//  for( uint32_t first = 0; first < movie_ids.size() - 1; ++first ) {
//    for( uint32_t second = first + 1; second < movie_ids.size(); ++second ) {
//      ibcf_matrix[first][second] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
//    }
//  }

  // Initialize all slots to be used (index of -1 means not used)

  similarity_vector.clear();

  for( uint32_t first = 0; first < dim; ++first ) {
    for( uint32_t second = 0; second < dim; ++second ) {
      movie_combo_lookup[first][second] = -1;
    }
  }
  
  // Slots initialized, add-in metrics.

  std::cerr << "Pass 1... " << std::endl;

  for( auto & u_m_r : Norm_train_users ) {    // Go through actual data set by user to get their item-item ratings.
    auto & m_r = u_m_r.second;                // Get this users movie/rating pairs.
    for( auto mit1 = m_r.begin(); mit1 != m_r.end(); ++mit1 ) {
      auto mit2 = mit1;
      std::advance(mit2, 1);
      uint16_t movie_id1 = (*mit1).first;
      float movie_rating1 = (*mit1).second;
      for( ; mit2 != m_r.end(); ++mit2 ) {	
	uint16_t movie_id2 = (*mit2).first;
	float movie_rating2 = (*mit2).second;
	uint32_t left_movie = (movie_id1 < movie_id2) ? (movie_id1) : (movie_id2);
	uint32_t right_movie = (movie_id1 < movie_id2) ? (movie_id2) : (movie_id1);
	int32_t lookup_index = movie_combo_lookup[left_movie][right_movie];
	if( lookup_index == -1 ) {
	  movie_combo_lookup[left_movie][right_movie] = similarity_vector.size();
	  lookup_index = similarity_vector.size();
	  similarity_vector.push_back( { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 } );
	}
	// !!!!! Important !!!!! vec.A must refer to the lower of the two indices.
	auto & vec = similarity_vector[lookup_index];
	if( pearson ) {
	  if( left_movie == movie_id1 ) {
	    vec.A_sum += movie_rating1;
	    vec.B_sum += movie_rating2;
	  } else {
	    vec.A_sum += movie_rating2;
	    vec.B_sum += movie_rating1;
	  }
	  vec.count += 1.0;
	}
	if( cosine ) {
	  float A = 0.0f;
	  float B = 0.0f;
	  if( left_movie == movie_id1 ) {
	    A = movie_rating1;
	    B = movie_rating2;
	  } else {
	    A = movie_rating2;
	    B = movie_rating1;
	  }
	  vec.numerator_AB += (A*B);
	  vec.denominator_A2 += (A*A);
	  vec.denominator_B2 += (B*B);
	}
      }
    }
  }

  // Now we can compute cosine similarities and be done with it.

  if( cosine ) {
    for( uint32_t first = 0; first < dim; ++first ) {
      for( uint32_t second = first+1; second < dim; ++second ) {
	int32_t lookup_index = movie_combo_lookup[first][second];
	auto & vec = similarity_vector[lookup_index];
	if( lookup_index != -1 ) {
	  float D = sqrt( vec.denominator_A2 ) * sqrt( vec.denominator_B2 );
	  vec.similarity = vec.numerator_AB / D;
	}
      }
    }
    return;
  }

  // Now just need to deal with Pearson which needs another couple of passes.

  std::cerr << "Pass 2 (Averages construction)... " << std::endl;
  
  for( uint32_t first = 0; first < dim; ++first ) {
    for( uint32_t second = first+1; second < dim; ++second ) {
      int32_t lookup_index = movie_combo_lookup[first][second];
      if( lookup_index != -1 ) {
	auto & vec = similarity_vector[lookup_index];
	vec.A_average = vec.A_sum / vec.count;
	vec.B_average = vec.B_sum / vec.count;
      }
    }
  }
  
  std::cerr << "Pass 3... " << std::endl;
  
  for( auto & u_m_r : Norm_train_users ) {    // Go through actual data set by user to get their item-item ratings.
    auto & m_r = u_m_r.second;                // Get this users movie/rating pairs.
    for( auto mit1 = m_r.begin(); mit1 != m_r.end(); ++mit1 ) {
      auto mit2 = mit1;
      std::advance(mit2, 1);
      uint16_t movie_id1 = (*mit1).first;
      float movie_rating1 = (*mit1).second;
      for( ; mit2 != m_r.end(); ++mit2 ) {	
	uint16_t movie_id2 = (*mit2).first;
	float movie_rating2 = (*mit2).second;
	uint32_t left_movie = (movie_id1 < movie_id2) ? (movie_id1) : (movie_id2);
	uint32_t right_movie = (movie_id1 < movie_id2) ? (movie_id2) : (movie_id1);
	int32_t lookup_index = movie_combo_lookup[left_movie][right_movie];
	
	// !!!!! Important !!!!! vec.A must refer to the lower of the two indices.
	
	auto & vec = similarity_vector[lookup_index];
	float adiff;
	float bdiff;
	if( left_movie == movie_id1 ) {
	  adiff = vec.A_average - movie_rating1;
	  bdiff = vec.B_average - movie_rating2;
	} else {
	  adiff = vec.A_average - movie_rating1;
	  bdiff = vec.B_average - movie_rating2;
	}
	vec.numerator_AB   += (adiff * bdiff);
	vec.denominator_A2 += (adiff * adiff);
	vec.denominator_B2 += (bdiff * bdiff);
      }
    }
  }

  std::cerr << "Pass 4 (Similarity construction)... " << std::endl;

  for( uint32_t first = 0; first < dim; ++first ) {
    for( uint32_t second = first+1; second < dim; ++second ) {
      int32_t lookup_index = movie_combo_lookup[first][second];
      if( lookup_index != -1 ) {
	auto & vec = similarity_vector[lookup_index];
	float D = sqrt( vec.denominator_A2 ) * sqrt( vec.denominator_B2 );
	vec.similarity = vec.numerator_AB / D;
      }
    }
  }

}
