library(recommenderlab)

# Notes:
# To get users with a certain number of ratings:
# >      library("recommenderlab")
# > data("MovieLense")
# > ### use only users with more than 100 ratings   !!!!!!!!!!
# > MovieLense100 <- MovieLense[rowCounts(MovieLense) >100,]  #
#  # !!!!!!!!!!! <- this can pre-filter to generate cold_starts vs. experienced users

# > MovieLense100
# 358 x 1664 rating matrix of class ‘realRatingMatrix’ with 73610 ratings.
# > train <- MovieLense100[1:50]

select_random_ratings <- function( user_ratings_list ) {
  random_length <- length(user_ratings_list[[1]])
  rnum <- sample(1:random_length,1)
  list_relem <- user_ratings_list[[1]][rnum]
}

all_movies_rated <- function( movie_averages ) {
  for( i in 1:movie_count ) {
    if( is.na(movies_averages[i]) ) {
      return(FALSE)
    }
  }
  return(TRUE)
}

set.seed(1234)

load = 1
if( load == 1 ) {
  df_movies <- read.csv("DF_M2000_U55000_S1234", header=TRUE)
  movies <- as(df_movies, "realRatingMatrix")
  movies_matrix <- as(df_movies, "matrix")
}
user_count = movies@data@Dim[1]
movie_count = movies@data@Dim[2]
movies_averages <- colMeans(movies)

# Select 60% for training, 40 % for testing.

#user_vector <- c(1:user_count)

train_set_vector <- sample(1:user_count, 0.6 * user_count)

#subset_vector <- c(2,10,25)
#other_subset_vector <- user_vector[! user_vector %in% subset_vector ]

movies_train <- movies[train_set_vector,]
movies_test  <- movies[-train_set_vector,]

movies_train_averages <- colMeans(movies_train)

#r <- Recommender(movies_train, method = "UBCF")

count <- 0
diff_sq_sum <- 0
for( user_test in 1:movies_test@data@Dim[1] ) {
  user_test_rating <- select_random_ratings( as(movies_test[user_test,], "list"))
  movie_test_name <- names(user_test_rating)
  user_movie_averages <- movies_train_averages[movie_test_name]
  if( is.na(user_movie_averages) ) {
    next
  }
  diff = user_test_rating[[1]] - user_movie_averages
  diff_sq = diff*diff
  diff_sq_sum <- diff_sq_sum + diff_sq
  count <- count + 1
  if( user_test %% 100 == 0 ) {
    rmse_m = sqrt(diff_sq_sum / count)
    message( "users processed = ", user_test, " rmse = ", rmse_m )
  }
}

rmse_m = sqrt(diff_sq_sum / count)
message( "Final rmse = ", rmse_m )
print("Done!")

# Final rmse = 1.0547491985419
