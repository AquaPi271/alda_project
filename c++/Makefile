
all: convert_to_dataframe generate_sample_movies read_netflix

read_netflix : bin/read_netflix
	@echo

generate_sample_movies : bin/generate_sample_movies
	#echo

convert_to_dataframe : bin/convert_to_dataframe
	@echo

bin/% : src/%.cc
	g++ -O3 -std=c++11 $< -o $@
