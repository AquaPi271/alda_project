
#all: convert_to_dataframe generate_sample_movies read_netflix days_since_2000 generate_all_movies \
#     generate_all_movies_binary \

INCLUDES = -I ./include

OBJECTS = objects/fold_generation.o \
          objects/load_data.o \
          objects/statistics.o

all: days_since_2000 generate_all_movies_binary generate_sample_movies_binary \
     read_netflix_binary base_metric ubcf_metric generate_sample_movies_binary_buckets \
     read_netflix_binary_densities generate_sample_movies_densities_binary ibcf_metric \

days_since_2000 : bin/days_since_2000
	@echo

generate_all_movies_binary : bin/generate_all_movies_binary
	@echo

generate_sample_movies_binary : bin/generate_sample_movies_binary
	@echo

generate_sample_movies_densities_binary : bin/generate_sample_movies_densities_binary
	@echo

generate_sample_movies_binary_buckets : bin/generate_sample_movies_binary_buckets
	@echo

read_netflix_binary : bin/read_netflix_binary
	@echo

read_netflix_binary_densities : bin/read_netflix_binary_densities
	@echo

base_metric : bin/base_metric
	@echo

ubcf_metric_normalized : bin/ubcf_metric_normalized
	@echo

ubcf_metric : bin/ubcf_metric
	@echo

ibcf_metric : bin/ibcf_metric
	@echo

bin/% : objects/%.o $(OBJECTS)
	g++ -O3 $(INCLUDES) -std=c++11 $^ -o $@

objects/%.o : src/%.cc include/*
	g++ -O3 -c $(INCLUDES) -std=c++11 $< -o $@

.PHONY : clean

clean :
	rm -f objects/*.o
	rm -f bin/*.exe
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



