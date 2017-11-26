#include "fold_generation.h"

#include <cmath>
#include <cstdint>
#include <vector>

void extract_fold_user_ids( const std::vector<uint32_t> & random_user_ids,
			    uint32_t fold,
			    uint32_t total_folds,
			    std::vector<uint32_t> & train_user_ids,
			    std::vector<uint32_t> & test_user_ids ) {

  float test_set_percent    = 1.0f / static_cast<float>( total_folds );
  float train_set_percent   = 1.0f - test_set_percent;
  uint32_t train_user_count = std::round( train_set_percent * random_user_ids.size() );
  uint32_t test_user_count  = random_user_ids.size() - train_user_count;

  // Get test fold start and end.
  uint32_t test_fold_start_index = test_set_percent * static_cast<float>(random_user_ids.size());
  uint32_t test_fold_end_index   = test_fold_start_index + test_user_count;

  for( uint32_t index = 0; index < random_user_ids.size(); ++index ) {
    if( (index >= test_fold_start_index) && (index < test_fold_end_index) ) {
      test_user_ids.push_back( random_user_ids[index] );
    } else {
      train_user_ids.push_back( random_user_ids[index] );
    }
  }
  
}

void extract_fold_user_ids_percentage_method( const std::vector<uint32_t> & random_user_ids,
					      float train_set_percent,
					      std::vector<uint32_t> & train_user_ids,
					      std::vector<uint32_t> & test_user_ids ) {
 
  uint32_t train_user_count = std::round( train_set_percent * random_user_ids.size() );
  uint32_t test_user_count = random_user_ids.size() - train_user_count;

  auto train_start = random_user_ids.begin();
  auto train_end   = random_user_ids.begin() + train_user_count;
  auto test_start  = random_user_ids.begin() + train_user_count;
  auto test_end    = random_user_ids.end();
  
  std::vector<uint32_t> train_user_ids_int(train_start, train_end);
  std::vector<uint32_t> test_user_ids_int(test_start, test_end);

  train_user_ids = train_user_ids_int;
  test_user_ids = test_user_ids_int;

  //std::cout << "train_user_count  = " << train_user_count << std::endl;
  //std::cout << "train vector size = " << train_user_ids.size() << std::endl;
  //std::cout << "test_user_count   = " << test_user_count << std::endl;
  //std::cout << "test vector size  = " << test_user_ids.size() << std::endl;

}
