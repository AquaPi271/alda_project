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

load = 0
if( load == 1 ) {
  df_movies <- read.csv("DF_SEED1234_0p1", header=TRUE)
  movies <- as(df_movies, "realRatingMatrix")
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

for( user_test in 1:movies_test@data@Dim[1] ) {
  user_test_rating <- select_random_ratings( as(movies_test[user_test,], "list"))
  movie_test_name <- names(user_test_rating)
  index <- 1
  for( m in names(movies_train_averages) ) {
    if( m == movie_test_name) {
      #print(movies_averages[[index]])
      break
    }
    index <- index + 1
  }
}

print("Done!")

# Let's randomly select a user.

#rm1 <- sample(1:user_count, 1)
#rm1_ratings <- as(movies[rm1,],"list")
#
#rm1_r1 <- select_random_ratings( rm1_ratings )
#rm1_name <- names(rm1_r1)
#
#index <- 1
#for( m in names(movies_averages) ) {
#  if( m == rm1_name) {
#    print("Found")
#    print(movies_averages[[index]])
#    break
#  }
#  index <- index + 1
#}
#rm1_rating <- rm1_r1[[1]]
#rmse_error <- sqrt((rm1_rating-movies_averages[[index]])**2/1)

# List functions

#print(length(rm1_ratings)) # Top level list length
#print(NROW(rm1_ratings)) # Top level list length
#print(NCOL(rm1_ratings)) # Top level list length
#print(rm1_ratings[[1]])  # Descend into first level of list 
#print(length(rm1_ratings[[1]]))  # Descend into first level of list 
#print(rm1_ratings[[1]][1])
#print(rm1_ratings[[1]])
#
#a_list <- list(
#  c(1,1,2,5,14,42),
#  month.abb,
#  matrix(c(3,-8,1,-3), nrow=2),
#  asin
#)
#
#a_list[[1]]

#rm1_d1 <- names(rm1_ratings[[1]][1])
