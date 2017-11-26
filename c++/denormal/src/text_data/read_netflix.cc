#include <algorithm>
#include <cmath>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

struct user_rating {
  uint32_t user_id;
  uint32_t rating;
};
    
auto read_movie_file( const std::string & filename,
		      std::map<uint32_t, std::map<uint32_t,uint32_t>> & movies,
		      std::map<uint32_t, std::map<uint32_t,uint32_t>> & users
		      ) -> void {

  std::ifstream file;
  std::string line;
  
  file.open(filename);
  std::getline( file, line );
  
  // Skip first line which is comment.

  std::getline( file, line );
  
  uint32_t movie;
  uint32_t user;
  uint32_t rating;
  uint32_t days;
  
  //  auto & user_map = movies[movie];
  //for( auto & u : user_map ) {
  //  std::cout << "user = " << u.first << " rating = " << u.second << std::endl;
  //}

  uint64_t count = 0;
  while( std::getline( file, line ) ) {
    std::string::size_type last_pos = 0;
    auto pos = line.find( ",", last_pos );
    movie = std::stoi(line.substr( 0, pos ));
    last_pos = pos + 1;
    pos = line.find( ",", last_pos );
    user = std::stoi(line.substr( last_pos, pos - last_pos ));
    last_pos = pos + 1;
    rating = std::stoi(line.substr( last_pos, pos - last_pos ));
    last_pos = pos + 1;
    days = std::stoi(line.substr( last_pos, pos - last_pos ));
    movies[movie][user] = rating;
    users[user][movie] = rating;
    ++count;
    if( count % 1000000 == 0 ) {
      std::cerr << count << std::endl;
    }
  }
  
  file.close();
  
}

auto compute_statistics( std::map<uint32_t, std::map<uint32_t,uint32_t>> & movies,
			 std::map<uint32_t, std::map<uint32_t,uint32_t>> & users ) -> void {

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
    std::cerr << "Usage: read_netflix <filename>" << std::endl;
    exit(1);
  }
  
  std::vector<std::string> file_list = {};
  std::string base_path{"./"};
  std::string s{ argv[1] };

  std::map<uint32_t,std::map<uint32_t,uint32_t>> movies;
  std::map<uint32_t, std::map<uint32_t,uint32_t>> users;
  
  read_movie_file( s, movies, users );
  compute_statistics( movies, users );
  
  return(0);

}
