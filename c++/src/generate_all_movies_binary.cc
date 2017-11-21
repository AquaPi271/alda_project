#include <algorithm>
#include <cstdlib>
#include <dirent.h>
//#include <fstream>
#include <iostream>
#include <map>
#include <fstream>
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

void write_uint32_to_buffer( char *buffer, uint32_t data, uint32_t index ) {
  buffer[index] = data & 0xFF;
  buffer[index+1] = (data >> 8) & 0xFF;
  buffer[index+2] = (data >> 16) & 0xFF;
  buffer[index+3] = (data >> 24) & 0xFF;
}

void write_uint16_to_buffer( char *buffer, uint16_t data, uint32_t index ) {
  buffer[index] = data & 0xFF;
  buffer[index+1] = (data >> 8) & 0xFF;
}

void write_uint8_to_buffer( char *buffer, uint8_t data, uint32_t index ) {
  buffer[index] = data & 0xFF;
}

uint32_t read_uint32_from_buffer( char *buffer, uint32_t index ) {
  uint32_t data =
    static_cast<uint8_t>(buffer[index]) |
    static_cast<uint8_t>(buffer[index+1]) << 8 |
    static_cast<uint8_t>(buffer[index+2]) << 16 |
    static_cast<uint8_t>(buffer[index+3]) << 24;
  return( data );
}

auto read_movie_file_to_binary( const std::string & filename,
				std::ofstream & outfile,
				char * buffer,
				uint32_t buffer_size ) -> void {

  std::ifstream file;
  std::string line;

  uint32_t bytes_in_buffer = 0;
  
  file.open(filename);
  std::getline( file, line );
  uint32_t movie = std::stoi( line );
  uint32_t index = 0;

  while( std::getline( file, line ) ) {
    std::string::size_type last_pos = 0;
    auto pos = line.find( ",", last_pos );
    auto user = std::stoi(line.substr( 0, pos - last_pos ));
    last_pos = pos+1;
    pos = line.find( ",", last_pos );
    auto rating = std::stoi(line.substr( last_pos, pos - last_pos ));
    last_pos = pos+1;
    pos = line.find( "-", last_pos );
    auto year = std::stoi(line.substr( last_pos, pos - last_pos ));
    last_pos = pos+1;
    pos = line.find( "-", last_pos );
    auto month = std::stoi(line.substr( last_pos, pos - last_pos ));
    last_pos = pos+1;
    auto day = std::stoi(line.substr( last_pos, pos - last_pos ));
    uint32_t days = compute_days_since_2000( month, day, year );
    if( days >= 65536 ) {
      std::cerr << "Days exceeds data size." << std::endl;
      exit(1);
    }

    uint32_t bytes_to_write = 2 + 4 + 1 + 2;
    if( bytes_to_write + bytes_in_buffer > buffer_size ) {
      outfile.write( buffer, bytes_in_buffer );
      bytes_in_buffer = 0;
      index = 0;
    }
    write_uint16_to_buffer( buffer, movie, index );
    write_uint32_to_buffer( buffer, user, index+2 );
    write_uint8_to_buffer( buffer, rating, index+6 );
    write_uint16_to_buffer( buffer, days, index+10 );
    index += bytes_to_write;
    bytes_in_buffer += bytes_to_write;

    //std::cout << movie << "," << user << "," << rating << "," << days << std::endl;
  }

  if( bytes_in_buffer > 0 ) {
    outfile.write( buffer, bytes_in_buffer );
  }
    
  file.close();
  
}

void read_file_list( std::vector<std::string> & file_list, const std::string & base_path ) {
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
    exit(1);
  }
}

auto main( int32_t argc, char ** argv ) -> int32_t {

  if( argc != 2 ) {
    std::cout << "Usage:  " << argv[0] << " <output_filename>" << std::endl;
    exit(1);
  }

  std::vector<std::string> file_list = {};
  std::string base_path{"./training_set/"};
  std::string output_filename{argv[1]};

  read_file_list( file_list, base_path );
  std::sort( file_list.begin(), file_list.end() );

  std::ofstream outfile;
  const uint32_t buffer_size = 9 * 1024;  // 4 uint32_t per entry each of 4 bytes for 1024 entries
  char buffer[buffer_size];
  outfile.open( output_filename, std::ios::binary | std::ios::out );
  
  auto count = 0;
  //std::cout << "movie,user,rating,days" << std::endl;
  for( auto & s : file_list ) {
    read_movie_file_to_binary( base_path + s, outfile, buffer, buffer_size );
    ++count;
    if( count % 100 == 0 ) {
      std::cerr << count << std::endl;
    }
  }

  outfile.close();
//
//  std::ifstream infile;
//  infile.open( output_filename, std::ios::binary | std::ios::in );
//  uint32_t amount_read = infile.readsome( buffer, buffer_size );
//  infile.close();
//
//  if( amount_read % 16 == 0 ) {
//    uint32_t movie  = read_bytes_from_buffer( buffer, 0 );
//    uint32_t user   = read_bytes_from_buffer( buffer, 4 );
//    uint32_t rating = read_bytes_from_buffer( buffer, 8 );
//    uint32_t days   = read_bytes_from_buffer( buffer, 12 );
//    std::cout << "movie = " << movie << std::endl;
//    std::cout << "user = " << user << std::endl;
//    std::cout << "rating = " << rating << std::endl;
//    std::cout << "days = " << days << std::endl;
//  }
//  //1488844
  return(0);

}
