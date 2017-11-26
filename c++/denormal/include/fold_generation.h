#pragma once

#include <cstdint>
#include <vector>

void extract_fold_user_ids( const std::vector<uint32_t> & random_user_ids,
			    uint32_t fold,
			    uint32_t total_folds,
			    std::vector<uint32_t> & train_user_ids,
			    std::vector<uint32_t> & test_user_ids );
void extract_fold_user_ids_percentage_method( const std::vector<uint32_t> & random_user_ids,
					      float train_set_percent,
					      std::vector<uint32_t> & train_user_ids,
					      std::vector<uint32_t> & test_user_ids );
