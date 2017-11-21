#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>
    
auto read_movie_file( const std::string & filename,
		      std::set<uint32_t> & movie_set,
		      std::set<uint32_t> & users_set ) -> void {

  std::ifstream file;
  std::string line;

  uint32_t movie;
  uint32_t user;
  uint32_t rating;
  
  file.open(filename);
  std::getline( file, line );
  
  // Skip first line which is comment.

  std::getline( file, line );

  uint32_t count = 0;
  while( std::getline( file, line ) ) {
    ++count;
    if( count % 1000000 == 0 ) {
      std::cerr << count << std::endl;
    }
    std::string::size_type last_pos = 0;
    auto pos = line.find( ",", last_pos );
    movie = std::stoi(line.substr( 0, last_pos - 1 ));
    if( movie_set.find(movie) == movie_set.end() ) {
      continue;
    }
    last_pos = pos + 1;
    pos = line.find( ",", last_pos );
    user = std::stoi(line.substr( last_pos, pos - last_pos + 1 ));
    if( users_set.find(user) == users_set.end() ) {
      continue;
    }
    last_pos = pos + 1;
    rating = std::stoi(line.substr( last_pos ));
    std::cout << movie << "," <<
      user << "," <<
      rating << std::endl;
  }
  
  file.close();
  
}
auto read_movie_file_for_ids( const std::string & filename,
			      std::set<uint32_t> & movie_set,
			      std::set<uint32_t> & ids_set ) -> void {

  std::ifstream infile;

  uint16_t movie;
  uint32_t user;
  uint8_t rating;
  
  file.open(filename);
  std::getline( file, line );
  
  // Skip first line which is comment.

  std::getline( file, line );

  uint32_t count = 0;
  while( std::getline( file, line ) ) {
    ++count;
    if( count % 1000000 == 0 ) {
      std::cerr << count << std::endl;
    }
    std::string::size_type last_pos = 0;
    auto pos = line.find( ",", last_pos );
    movie = std::stoi(line.substr( 0, last_pos - 1 ));
    if( movie_set.find(movie) == movie_set.end() ) {
      continue;
    }
    last_pos = pos + 1;
    pos = line.find( ",", last_pos );
    user = std::stoi(line.substr( last_pos, pos - last_pos + 1 ));
    ids_set.insert( user );
  }
  
  file.close();
  
}

auto read_movie_file_for_ids( const std::string & filename,
			      std::set<uint32_t> & movie_set,
			      std::set<uint32_t> & ids_set ) -> void {

  std::ifstream file;
  std::string line;

  uint32_t movie;
  uint32_t user;
  uint32_t rating;
  
  file.open(filename);
  std::getline( file, line );
  
  // Skip first line which is comment.

  std::getline( file, line );

  uint32_t count = 0;
  while( std::getline( file, line ) ) {
    ++count;
    if( count % 1000000 == 0 ) {
      std::cerr << count << std::endl;
    }
    std::string::size_type last_pos = 0;
    auto pos = line.find( ",", last_pos );
    movie = std::stoi(line.substr( 0, last_pos - 1 ));
    if( movie_set.find(movie) == movie_set.end() ) {
      continue;
    }
    last_pos = pos + 1;
    pos = line.find( ",", last_pos );
    user = std::stoi(line.substr( last_pos, pos - last_pos + 1 ));
    ids_set.insert( user );
  }
  
  file.close();
  
}

auto main( int32_t argc, char **argv ) -> int {
  
  // Awesome, R can handle the overall sampling.  Just need to supply the data.
  
  if( argc != 4 ) {
    std::cerr << "Usage:  generate_sample_movies <seed> <movie_count> <user_count>" << std::endl;
    exit(1);
  }

  std::string movie_filename{ "ALL_MOVIES" };
  std::string seed_text{ argv[1] };
  std::string movie_count_text{ argv[2] };
  std::string user_count_text{ argv[3] };
  
  uint32_t seed = std::stoi( seed_text );
  std::srand( seed );
  uint32_t movie_count = std::stoi( movie_count_text );
  uint32_t user_count = std::stoi( user_count_text );
  
  if( movie_count < 1 || movie_count > 17700 ) {
    std::cerr << "Error:  Movie count must be between 1 and 17700 but is " <<
      movie_count << std::endl;
    exit(1);
  }
  if( user_count < 1 || user_count > 480189 ) {
    std::cerr << "Error:  User count must be between 1 and 480189 but is " <<
      user_count << std::endl;
    exit(1);
  }

  std::vector<uint32_t> movies_selected = {};
  for( auto i = 1; i <= 17700; ++i ) {
    movies_selected.push_back(i);
  }

  std::random_shuffle(movies_selected.begin(), movies_selected.end());
  std::set<uint32_t> random_movies = {};
  for( auto i = 1; i <= movie_count; ++i ) {
    random_movies.insert( movies_selected[i] );
  }

  std::set<uint32_t> users = {};
  read_movie_file_for_ids(movie_filename, random_movies, users);

  std::vector<uint32_t> valid_user_ids = {};
  for( auto & u : users ) {
    valid_user_ids.push_back(u);
  }

  std::random_shuffle(valid_user_ids.begin(), valid_user_ids.end());

  std::set<uint32_t> users_selected = {};
  for( auto i = 1; i <= user_count; ++i ) {
    users_selected.insert( valid_user_ids[i] );
  }
  
  read_movie_file( movie_filename, random_movies, users_selected );
  
  return(0);

}
