
#all: convert_to_dataframe generate_sample_movies read_netflix days_since_2000 generate_all_movies \
#     generate_all_movies_binary \

all: days_since_2000 generate_all_movies_binary generate_sample_movies_binary

days_since_2000 : bin/days_since_2000
	@echo

generate_all_movies_binary : bin/generate_all_movies_binary
	@echo

generate_sample_movies_binary : bin/generate_sample_movies_binary
	@echo

bin/% : src/%.cc
	g++ -O3 -std=c++11 $< -o $@

.PHONY : clean

clean :
	find . -name "*~" -print | xargs rm -f

#####################

generate_all_movies : bin/generate_all_movies
	@echo

generate_sample_movies : bin/generate_sample_movies
	@echo

read_netflix : bin/read_netflix
	@echo

convert_to_dataframe : bin/convert_to_dataframe
	@echo



