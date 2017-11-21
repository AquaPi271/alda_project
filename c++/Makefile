
all: convert_to_dataframe generate_sample_movies read_netflix days_since_2000 generate_all_movies

days_since_2000 : bin/days_since_2000
	@echo

read_netflix : bin/read_netflix
	@echo

generate_sample_movies : bin/generate_sample_movies
	@echo

generate_all_movies : bin/generate_all_movies
	@echo

convert_to_dataframe : bin/convert_to_dataframe
	@echo

bin/% : src/%.cc
	g++ -O3 -std=c++11 $< -o $@

.PHONY : clean

clean :
	find . -name "*~" -print | xargs rm -f
