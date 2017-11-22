#pragma once

#include <cstdint>
#include <map>

auto compute_bulk_statistics( std::map<uint16_t, std::map<uint32_t,uint8_t>> & movies,
			      std::map<uint32_t, std::map<uint16_t,uint8_t>> & users ) -> void;
