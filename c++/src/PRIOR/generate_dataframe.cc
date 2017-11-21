#include <algorithm>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

auto read_movie_file( const std::string & filename ) -> void {

  std::ifstream file;
  std::string line;

  file.open(filename);
  std::getline( file, line );
  auto movie = std::stoi( line );

  while( std::getline( file, line ) ) {
  
    std::string::size_type last_pos = 0;
    auto pos = line.find( ",", last_pos );
    std::cout << movie << "," << line.substr( 0, pos - last_pos + 1);
    last_pos = pos+1;
    pos = line.find( ",", last_pos );
    std::cout << line.substr( last_pos, pos - last_pos ) << std::endl;
  }
  
  file.close();
  
}

auto main( void ) -> int {

  std::vector<std::string> file_list = {};
  std::string base_path{"./training_set/"};

  DIR *dir = nullptr;
  struct dirent *ent = nullptr;

  if( (dir = opendir(base_path.c_str())) != nullptr ) {
    while( (ent = readdir(dir)) != NULL ) {
      std::string ent_file{ent->d_name};
      if( ent_file.length() >= 4 ) {
	if( ent_file.substr( ent_file.length() - 4, 4 ) == ".txt" ) {
	  file_list.push_back( ent_file );
	}
      }
    }
    closedir(dir);
  } else {
    std::cerr << "Failed to open directory" << std::endl;
    return(1);
  }

  std::sort( file_list.begin(), file_list.end() );

  auto count = 0;
  
  std::cout << "movie,user,rating" << std::endl;
  for( auto & s : file_list ) {
    read_movie_file( base_path + s );
    ++count;
    if( count % 100 == 0 ) {
      std::cerr << count << std::endl;
    }
  }
  
  return(0);

}
