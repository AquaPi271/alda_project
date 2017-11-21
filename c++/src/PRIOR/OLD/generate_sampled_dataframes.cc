#include <algorithm>
#include <cmath>
#include <cstdint>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

auto read_movie_file_percentage( const std::string & filename, float percentage ) -> void {

  std::ifstream file;
  std::string line;
  std::vector<std::string> lines = {};

  file.open(filename);
  std::getline( file, line );
  auto movie = std::stoi( line );
  std::string movie_string { std::to_string(movie) };

  while( std::getline( file, line ) ) {
    std::string::size_type last_pos = 0;
    auto pos = line.find( ",", last_pos );
    std::string mline = "u" + line.substr( 0, pos - last_pos + 1) + "i" + movie_string + ",";
    last_pos = pos+1;
    pos = line.find( ",", last_pos );
    mline += line.substr( last_pos, pos - last_pos );
    lines.push_back(mline);
  }
  
  file.close();

  uint32_t elements = lines.size();
  uint32_t select_count = round( static_cast<float>(elements) * percentage );
  if( select_count == 0 ) {
    std::cerr << "Movie " << movie << " has no selection counts, bumping to 1." << std::endl;
    select_count = 1;
  }
  std::random_shuffle( lines.begin(), lines.end() );
  for( uint32_t index = 0; index < select_count; ++index ) {
    std::cout << lines[index] << std::endl;
  }

}

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

auto retrieve_txt_filelist( std::vector<std::string> * const file_list,
			    const std::string base_path ) -> void {
  DIR *dir = nullptr;
  struct dirent *ent = nullptr;

  if( (dir = opendir(base_path.c_str())) != nullptr ) {
    while( (ent = readdir(dir)) != NULL ) {
      std::string ent_file{ent->d_name};
      if( ent_file.length() >= 4 ) {
	if( ent_file.substr( ent_file.length() - 4, 4 ) == ".txt" ) {
	  file_list->push_back( ent_file );
	}
      }
    }
    closedir(dir);
  } else {
    std::cerr << "Failed to open directory" << std::endl;
  }
  
}

auto main( int32_t argc, char **argv ) -> int {

  uint32_t seed = 1234;
  std::srand( seed );
  
  // Awesome, R can handle the overall sampling.  Just need to supply the data.
  
  if( argc != 2 ) {
    std::cerr << "Usage:  generate_sampled_dataframes <percentage>" << std::endl;
    exit(1);
  }
  
  std::string percentage_text{argv[1]};
  float percentage = std::stof(percentage_text);
  if( percentage < 0 || percentage > 1 ) {
    std::cerr << "Error:  The given percentage must be between 0 and 1." << std::endl;
    exit(1);
  }
  
  std::vector<std::string> file_list = {};
  std::string base_path{"./training_set/"};
  auto count = 0;

  retrieve_txt_filelist( &file_list, base_path );
  std::sort( file_list.begin(), file_list.end() );

  std::cout << "user,movie,rating" << std::endl;
  for( auto & s : file_list ) {
    read_movie_file_percentage( base_path + s, percentage );
    ++count;
    if( count % 100 == 0 ) {
      std::cerr << count << std::endl;
    }
  }
  
  return(0);

}
