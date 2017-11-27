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

//const uint32_t dim = 18000;
//const uint32_t dim = 100;
//static int32_t movie_combo_lookup[dim][dim] = {};
//std::vector<similarity_info> similarity_vector = {};
//const float default_no_similarity = -1000.0f;

const float abs_threshold = 0.0001;
const float bad_similarity = -1000.0f;

auto main( int argc, char **argv ) -> int {

  if( argc != 8 ) {
    std::cerr << "Usage: " << argv[0] << " <filename> <random_seed> <folds> <train_test_percent> <k> <similarity_method> <normalization>"
	      << std::endl;
    exit(1);
  }

  std::string s{ argv[1] };
  uint32_t    random_seed       = std::stoi( argv[2] );
  uint32_t    folds             = std::stoi( argv[3] );
  float       train_set_percent = std::stof( argv[4] );
  uint32_t    k                 = std::stoi( argv[5] );
  std::string method_arg{ argv[6] };
  uint32_t    normalization     = std::stoi( argv[7] );
  bool normalize = false;
  if( normalization == 1 ) {
    normalize = true;
  }
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

    normalize_from( train_users, Norm_train_users, normalize );
    normalize_from( test_users, Norm_test_users, normalize );

    uint32_t movie_count = train_movie_users.size();
    uint32_t user_count = Norm_train_users.size();
    
    float ** movie_movie = new float * [movie_count];
    for( uint32_t i = 0; i < movie_count; ++i ) {
      movie_movie[i] = new float[movie_count];
      for( uint32_t j = 0; j < movie_count; ++j ) {
	movie_movie[i][j] = 0.0f;
      }
    }
    
    // Build similarity item-item look-ups.
    
    std::map<uint16_t,uint32_t> movie_to_index_map = {};

    build_item_item_matrix( Norm_train_users, user_count, movie_count, movie_to_index_map, movie_movie, cosine, pearson );
    
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
      
      std::vector<nn> matched_movies = {};
      
      //    if( find_top_knn_users_cosine( k, train_movie_users, train_users, test_users,
      //				   tu.first, test_movie,
      //				   matched_users ) ) {
      bool warm_reading = false;
      if( cosine ) {
	warm_reading = find_top_knn_movies_cosine_normalized( k, Norm_test_users, tu.first, test_movie, movie_to_index_map, movie_movie, matched_movies );
      } else if( pearson ) {
	warm_reading = find_top_knn_movies_pearson_normalized( k, Norm_test_users, tu.first, test_movie, movie_to_index_map, movie_movie, matched_movies );
      }
      if( warm_reading ) {
	// Get matched movies  ratings.
	float ratings_sum = 0;
	float ratings_count = 0;

	float similarity_sum = 0.0;
	for( auto & mm : matched_movies ) {
	  similarity_sum += mm.dist;
	}

	if( std::fabs( similarity_sum ) < abs_threshold ) {	
	  for( auto & mm : matched_movies ) {  
	    ratings_sum += (static_cast<float>(test_users[tu.first][mm.movie_id]));
	    ratings_count += 1.0f;
	  }
	} else {
	  for( auto & mm : matched_movies ) {
	    // Get the rating that the test user had for this movie, do not need to discount bias because built-in for user.	  
	    ratings_sum += (static_cast<float>(test_users[tu.first][mm.movie_id]) * (mm.dist) / similarity_sum);
	    ratings_count += 1.0f;
	  }
	}
	
	
	/*	float similarity_sum_squared = 0.0;
	for( auto & mm : matched_movies ) {
	  similarity_sum_squared += (mm.dist * mm.dist);
	}
	
	for( auto & mm : matched_movies ) {
	  // Get the rating that the test user had for this movie, do not need to discount bias because built-in for user.	  
	  ratings_sum += (static_cast<float>(test_users[tu.first][mm.movie_id]) * (mm.dist*mm.dist) / similarity_sum_squared);
	  ratings_count += 1.0f;
	  }*/
	
	if( ratings_count <= 0.0f ) {
	  auto test_rating = static_cast<float>((*item).second);
	  auto test_movie = (*item).first;
	  auto avg_rating = train_movie_averages[test_movie];
	  rmse_N += ((test_rating - avg_rating)*(test_rating - avg_rating));
	  ++rmse_D;
	} else {
	  float rating = ratings_sum;   // NOTE: bias not needed here because using own ratings.
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

    if( std::fabs(rmse_D) > abs_threshold ) {
      RMSE[fold] = sqrt( rmse_N / rmse_D );
      std::cerr << "RMSE = " << RMSE[fold] << "  k = " << k << " fold = " << fold << " cold start count = " << cold_start_count << " / " << count << std::endl;
    } else {
      RMSE[fold] = 1.05;
    }

    // Free up movie_movie array.
    
    for( uint32_t i = 0; i < movie_count; ++i ) {
      delete [] movie_movie[i];
    }
    delete [] movie_movie;
  
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
  std::map<uint16_t,uint32_t> & movie_to_index_map,
  float ** movie_movie,
  std::vector<nn> & matched_movies ) {

  // FIRST:  Get only the movies that this user watched.  From this list choose the best k.
  // I messed this up earlier.

  std::vector<uint16_t> movies_watched_by_test = {};
  for( auto & m : test_users[user_id] ) {
    if( m.first != requested_movie ) {
      movies_watched_by_test.push_back( m.first );
    }
  }

  uint32_t test_movie_index = movie_to_index_map[requested_movie];
  for( auto & m : movies_watched_by_test ) {
    uint32_t compare_movie_index = movie_to_index_map[m];
    float dist = movie_movie[test_movie_index][compare_movie_index];
    add_to_knn( k, m, dist, matched_movies );
  }
  
  if( matched_movies.size() < 1 ) {
    return(false);
  }
  
  return( true );
}

bool find_top_knn_movies_pearson_normalized
( uint32_t k,  
  std::map<uint32_t,std::map<uint16_t,float>> & test_users,
  uint32_t user_id,
  uint32_t requested_movie,
  std::map<uint16_t,uint32_t> & movie_to_index_map,
  float ** movie_movie,
  std::vector<nn> & matched_movies ) {

  // FIRST:  Get only the movies that this user watched.  From this list choose the best k.
  // I messed this up earlier.

  std::vector<uint16_t> movies_watched_by_test = {};
  for( auto & m : test_users[user_id] ) {
    if( m.first != requested_movie ) {
      movies_watched_by_test.push_back( m.first );
    }
  }

  uint32_t test_movie_index = movie_to_index_map[requested_movie];
  for( auto & m : movies_watched_by_test ) {
    uint32_t compare_movie_index = movie_to_index_map[m];
    float dist = movie_movie[test_movie_index][compare_movie_index];
    add_to_knn( k, m, dist, matched_movies );
  }
  
  if( matched_movies.size() < 1 ) {
    return(false);
  }
  
  return( true );
}

void add_to_knn( uint32_t k, uint32_t wmovie_id, float dist, std::vector<nn> & knn ) {

  //  std::cout << dist << " " << " knn.size() = " << knn.size() << std::endl;

  if( dist > bad_similarity ) {
  
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
  
}

void normalize_from( std::map<uint32_t,std::map<uint16_t,uint8_t>> & u_m_r,
		     std::map<uint32_t,std::map<uint16_t,float>> & u_m_r_OUT,
		     bool normalize ) {
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
      if( normalize ) {
	u_m_r_OUT[u.first][m.first] = static_cast<float>( m.second ) - average;
      } else {
	u_m_r_OUT[u.first][m.first] = static_cast<float>( m.second );
      }
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

  if( std::fabs(N) < abs_threshold ) {
    return( 0.0f );
  }
  
  if( std::fabs(N) < abs_threshold ) {
    return( 0.0f );
  }
  
  return( bias / N );
  
}

void build_item_item_matrix( std::map<uint32_t,std::map<uint16_t,float>> & Norm_train_users,      // Normalized ratings u_m_r
			     uint32_t user_count,
			     uint32_t movie_count,
			     std::map<uint16_t,uint32_t> & movie_to_index_map,
			     float ** movie_movie,			     
			     bool cosine,
			     bool pearson ) {
  
  // Allocate matrix.

  uint32_t rows = movie_count;
  uint32_t columns = user_count;

  //float ** user_movie = new float * [rows];
  //for( uint32_t i = 0; i < rows; ++i ) {
  //  user_movie[i] = new float[columns];
  //  for( uint32_t j = 0; j < columns; ++j ) {
  //    user_movie[i][j] = 0.0f;
  //  }
  //}

  //  float * user_movie1D = new float[rows * columns];
  
  float * movie_user1D = new float[rows * columns];
  for(uint32_t i = 0; i < rows; ++i) {  // movie
    for(uint32_t j = 0; j < columns; ++j ) { // users
      movie_user1D[i*columns + j] = 0.0f;
    }
  }

  
  // Do not need to keep track of user ids, only movie ids.

  //std::map<uint32_t,uint16_t> index_to_movie_map = {};
  
  uint32_t user_index = 0;
  uint32_t movie_index = 0;
  for( auto & u_m_r : Norm_train_users ) {
    for( auto & m_r : u_m_r.second ) {
      auto mit = movie_to_index_map.find( m_r.first );
      if( mit == movie_to_index_map.end() ) {
	movie_to_index_map[m_r.first] = movie_index;
	//index_to_movie_map[movie_index] = m_r.first;
	//user_movie[user_index][movie_index] = m_r.second;  // Place rating in matrix.
	//movie_user1D[user_index*movie_count+movie_index] = m_r.second;  // Place rating in matrix.
	movie_user1D[movie_index*user_count+user_index] = m_r.second;  // Place rating in matrix.
	++movie_index;
      } else {
	//user_movie[user_index][(*mit).second] = m_r.second;  // Place rating in matrix.	
	//user_movie1D[user_index*movie_count + (*mit).second] = m_r.second;  // Place rating in matrix.		
	movie_user1D[(*mit).second*user_count + user_index ] = m_r.second;  // Place rating in matrix.	
      }
    }
    ++user_index;
  }
	   
  for( uint32_t movie_index_left = 0; movie_index_left < movie_count - 1; ++movie_index_left ) {
    for( uint32_t movie_index_right = movie_index_left + 1; movie_index_right < movie_count; ++movie_index_right ) {
      float A_sum = 0.0f;
      float B_sum = 0.0f;
      float count = 0.0f;
      float AB_sum = 0.0f;
      float A2_sum = 0.0f;
      float B2_sum = 0.0f;
      float similarity = 0.0f;
      for( uint32_t user_index = 0; user_index < user_count; ++user_index ) {
	//float A = user_movie[user_index][movie_index_left];
	//float B = user_movie[user_index][movie_index_right];
	//float A = user_movie1D[user_index*movie_count+movie_index_left];
	//float B = user_movie1D[user_index*movie_count+movie_index_right];
	float A = movie_user1D[movie_index_left*user_count+user_index];
	float B = movie_user1D[movie_index_right*user_count+user_index];
	if( (A == 0.0f) || (B == 0.0f) ) { continue; }
	A_sum += A;
	B_sum += B;
	AB_sum += (A*B);
	A2_sum += (A*A);
	B2_sum += (B*B);
	count += 1.0f;
      }
      if( cosine ) {
	float D = sqrt(A2_sum) * sqrt(B2_sum);
	if( std::fabs(D) > abs_threshold ) {
	  similarity = AB_sum / D;
	} else {
	  similarity = -1000.0f;
	}
      } else if( pearson ) {
	if( count < 1.0f ) {
	  similarity = -1000.0f;
	} else {
	  float A_avg = A_sum / count;
	  float B_avg = B_sum / count;      
	  float N_sum = 0.0f;
	  A2_sum = 0.0f;
	  B2_sum = 0.0f;
	  for( uint32_t user_index = 0; user_index < user_count; ++user_index ) {
	    //float A = user_movie[user_index][movie_index_left];
	    //float B = user_movie[user_index][movie_index_right];
	    //float A = user_movie1D[user_index*movie_count+movie_index_left];
	    //float B = user_movie1D[user_index*movie_count+movie_index_right];
	    float A = movie_user1D[movie_index_left*user_count+user_index];
	    float B = movie_user1D[movie_index_right*user_count+user_index];
	    if( (A == 0.0f) || (B == 0.0f) ) { continue; }
	    float A_diff = (A - A_avg);
	    float B_diff = (B - B_avg);
	    N_sum += (A_diff * B_diff);
	    A2_sum += (A_diff * A_diff);
	    B2_sum += (B_diff * B_diff);
	  }
	  float D = sqrt(A2_sum)*sqrt(B2_sum);
	  if( std::fabs(D) > abs_threshold ) {
	    similarity = N_sum / D;
	  } else {
	    similarity = -1000.0f;
	  }
	}
      }
      
      // Similarity is now computed for Pearson or Cosine, now need to record it.
      
      movie_movie[movie_index_left][movie_index_right] = similarity;
      movie_movie[movie_index_right][movie_index_left] = similarity;
    }
  }
    
  // Free up resources.

  //delete [] user_movie1D;
  delete [] movie_user1D;
  
  //for( uint32_t i = 0; i < rows; ++i ) {
  //  delete [] user_movie[i];
  //}
  //delete [] user_movie;
  
}
