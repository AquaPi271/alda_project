#include <cstdlib>
#include <iostream>
#include <map>

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
  if( (month < 1) || (month > 12) || (year < 2000) || (day < 1) || (day > 31) ) {
    std::cerr << "Bad date:  " << month << " / " << day << " / " << year << std::endl;
    exit(1);
  }
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

int32_t main( int32_t argc, char **argv ) {

  std::cout << "Is it a leap year?" << std::endl;
  std::cout << "2000:  " << is_leap_year(2000) << std::endl;
  std::cout << "1900:  " << is_leap_year(1900) << std::endl;
  std::cout << "2001:  " << is_leap_year(2001) << std::endl;
  std::cout << "2002:  " << is_leap_year(2002) << std::endl;
  std::cout << "2003:  " << is_leap_year(2003) << std::endl;
  std::cout << "2004:  " << is_leap_year(2004) << std::endl;

  std::cout << "Days since 1/1/2000: " << std::endl;
  std::cout << "(1/1/2000)   => " << compute_days_since_2000(1,1,2000) << std::endl;
  std::cout << "(1/1/2001)   => " << compute_days_since_2000(1,1,2001) << std::endl;
  std::cout << "(9/18/2010)  => " << compute_days_since_2000(9,18,2010) << std::endl;
  
}
