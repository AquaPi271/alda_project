#include <algorithm>
#include <cmath>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <vector>

const float test_set_percent = 0.90;

void read_record( char * buffer, uint32_t index,
		  uint16_t * movie,
		  uint32_t * user,
		  uint8_t  * rating,
		  uint16_t * days );
uint32_t read_uint32_from_buffer( char *buffer, uint32_t index );
uint16_t read_uint16_from_buffer( char *buffer, uint32_t index );
uint8_t  read_uint8_from_buffer( char *buffer, uint32_t index );
void read_movie_file_binary( const std::string & filename,
			     std::map<uint16_t, std::map<uint32_t,uint8_t>> & movies,
			     std::map<uint32_t, std::map<uint16_t,uint8_t>> & users
			     );
bool find_top_knn_users( std::map<uint16_t,std::map<uint32_t,uint8_t>> & train_movie_users,
			 std::map<uint32_t,std::map<uint16_t,uint8_t>> & train_users_movies,
			 std::map<uint32_t,std::map<uint16_t,uint8_t>> & test_users,
			 uint32_t user_id,
			 uint32_t requested_movie );

const uint32_t record_size = 9;
const uint32_t buffer_size = record_size * 1024;
char buffer[buffer_size];

void read_record( char * buffer, uint32_t index,
		  uint16_t * movie,
		  uint32_t * user,
		  uint8_t  * rating,
		  uint16_t * days ) {
  *movie  = read_uint16_from_buffer( buffer, index );
  *user   = read_uint32_from_buffer( buffer, index+2 );
  *rating = read_uint8_from_buffer(  buffer, index+6 );
  *days   = read_uint16_from_buffer( buffer, index+7 );
}

uint32_t read_uint32_from_buffer( char *buffer, uint32_t index ) {
  uint32_t data =
    static_cast<uint8_t>(buffer[index]) |
    static_cast<uint8_t>(buffer[index+1]) << 8 |
    static_cast<uint8_t>(buffer[index+2]) << 16 |
    static_cast<uint8_t>(buffer[index+3]) << 24;
  return( data );
}

uint16_t read_uint16_from_buffer( char *buffer, uint32_t index ) {
  uint16_t data =
    static_cast<uint8_t>(buffer[index]) |
    static_cast<uint8_t>(buffer[index+1]) << 8;
  return( data );
}

uint8_t read_uint8_from_buffer( char *buffer, uint32_t index ) {
  uint8_t data =
    static_cast<uint8_t>(buffer[index]);
  return( data );
}

auto read_movie_file_binary( const std::string & filename,
			     std::map<uint16_t, std::map<uint32_t,uint8_t>> & movies,
			     std::map<uint32_t, std::map<uint16_t,uint8_t>> & users
			     ) -> void {
  
  std::ifstream infile;
  uint32_t bytes_read = 0;
  uint16_t movie;
  uint32_t user;
  uint8_t rating;
  uint16_t days;
  
  infile.open( filename, std::ios::binary | std::ios::in | std::ios::ate );
  if( !infile.is_open() ) {
    std::cout << "Could not open file " << filename << std::endl;
    exit(1);
  }
  uint64_t file_size = infile.tellg();
  infile.seekg(0, std::ios::beg);

  while( bytes_read < file_size ) {
    uint32_t amount_read = infile.readsome( buffer, buffer_size );
    bytes_read += amount_read;
    if( amount_read % 9 != 0 ) {
      std::cerr << "Badness has happened.  Non-multiple of record size" << std::endl;
      exit(1);
    }
    uint32_t records_read = amount_read / record_size;
    for( uint32_t r = 0; r < records_read; ++r ) {
      read_record( buffer, r * record_size, &movie, &user, &rating, &days );
      movies[movie][user] = rating;
      users[user][movie] = rating;
    }
  } 
  infile.close();
}

auto compute_statistics( std::map<uint16_t, std::map<uint32_t,uint8_t>> & movies,
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

  

auto main( int argc, char **argv ) -> int {

  if( argc != 2 ) {
    std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
    exit(1);
  }

  std::default_random_engine random_generator{1};

  std::vector<std::string> file_list = {};
  std::string base_path{"./"};
  std::string s{ argv[1] };

  std::map<uint16_t,std::map<uint32_t,uint8_t>> movies;
  std::map<uint32_t,std::map<uint16_t,uint8_t>> users;
  
  read_movie_file_binary( s, movies, users );

  // Spaghetti for now.

  std::vector<uint32_t> random_user_ids = {};
  for( auto & u : users ) {
    random_user_ids.push_back( u.first );
  }
  
  std::random_shuffle( random_user_ids.begin(), random_user_ids.end() );

  uint32_t train_user_count = std::round( test_set_percent * users.size() );
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

    find_top_knn_users( train_movie_users, train_users, test_users, tu.first, test_movie ); 
    //rmse_N += ((test_rating - avg_rating)*(test_rating - avg_rating));
    //++rmse_D;
    
  }

  //std::cout << "RMSE = " << sqrt( rmse_N / rmse_D ) << std::endl;
  
  return(0);

}

bool find_top_knn_users( std::map<uint16_t,std::map<uint32_t,uint8_t>> & train_movie_users,
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
    float D = sqrt(A2) * sqrt(B2);
    if( D != 0 ) {
      auto cos_dist = static_cast<float>(AB) / D;
    }
  }

  return( true );
}
