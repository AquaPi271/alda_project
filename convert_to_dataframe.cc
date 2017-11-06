#include <algorithm>
#include <cmath>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

auto convert_movie_file( const std::string & filename ) -> void {

  std::ifstream file;
  std::string line;
  
  file.open(filename);
  std::getline( file, line );
  
  // Skip first line which is comment.

  std::getline( file, line );
  
  uint32_t movie;
  uint32_t user;
  uint32_t rating;

  //  auto & user_map = movies[movie];
  //for( auto & u : user_map ) {
  //  std::cout << "user = " << u.first << " rating = " << u.second << std::endl;
  //}

  uint64_t count = 0;
  while( std::getline( file, line ) ) {
    std::string::size_type last_pos = 0;
    auto pos = line.find( ",", last_pos );
    movie = std::stoi(line.substr( 0, last_pos - 1 ));
    last_pos = pos + 1;
    pos = line.find( ",", last_pos );
    user = std::stoi(line.substr( last_pos, pos - last_pos + 1 ));
    last_pos = pos + 1;
    rating = std::stoi(line.substr( last_pos ));
    std::cout << "u" << user << ",i" << movie << "," << rating << std::endl;
    ++count;
    if( count % 1000000 == 0 ) {
      std::cerr << count << std::endl;
    }
  }
  
  file.close();
  
}

auto main( int argc, char **argv ) -> int {

  if( argc != 2 ) {
    std::cerr << "Usage: convert_to_dataframe <filename>" << std::endl;
    exit(1);
  }
  
  std::string s{ argv[1] };

  std::map<uint32_t,std::map<uint32_t,uint32_t>> movies;
  std::map<uint32_t, std::map<uint32_t,uint32_t>> users;

  std::cout << "user,movie,rating" << std::endl;
  convert_movie_file( s );
  
  return(0);

}
