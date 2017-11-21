#include <algorithm>
#include <cstdlib>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

std::map<int32_t,int32_t> month_lookup =
  { {1,31}, {2,28}, {3,31}, {4,30}, {5,31}, {6,30},
    {7,31}, {8,31}, {9,30}, {10,31}, {11,30}, {12,31} };

bool is_leap_year( uint32_t year ) {
  if( year % 4 != 0 ) { return( false ); }
  if( year % 100 != 0 ) { return( true ); }
  if( year % 400 != 0 ) { return( false ); }
  return( true );
}

int32_t compute_days_since_2000( int32_t month, int32_t day, int32_t year ) {
  int32_t leap_years = 0;
  int32_t non_leap_years = 0;
  int32_t diff_years = 0;
  int32_t sum_days = 0;
  // First compute full years since 2000.
  diff_years = year - 2000;
  if( diff_years > 0 ) {
    leap_years = (diff_years-1) / 4 + 1;
    non_leap_years = diff_years - leap_years;
  }
  sum_days = leap_years * 366 + non_leap_years * 365;
  // Full months since January.
  if( month > 1 ) {
    for( int32_t s_month = 1; s_month < month ; ++s_month ) {
      sum_days += month_lookup[s_month];
    }
  }
  sum_days += ( day - 1 );
  if( is_leap_year( year ) && (month > 2) ) { ++sum_days; }
  return(sum_days);
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
    auto user = line.substr( 0, pos - last_pos );
    last_pos = pos+1;
    pos = line.find( ",", last_pos );
    auto rating = line.substr( last_pos, pos - last_pos );
    last_pos = pos+1;
    pos = line.find( "-", last_pos );
    auto year = std::stoi(line.substr( last_pos, pos - last_pos ));
    last_pos = pos+1;
    pos = line.find( "-", last_pos );
    auto month = std::stoi(line.substr( last_pos, pos - last_pos ));
    last_pos = pos+1;
    auto day = std::stoi(line.substr( last_pos, pos - last_pos ));
    auto days = compute_days_since_2000( month, day, year );

    std::cout << movie << "," << user << "," << rating << "," << days << std::endl;
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
  
  std::cout << "movie,user,rating,days" << std::endl;
  for( auto & s : file_list ) {
    read_movie_file( base_path + s );
    ++count;
    if( count % 100 == 0 ) {
      std::cerr << count << std::endl;
    }
  }
  
  return(0);

}
