library(recommenderlab)

set.seed(1234)

df_movies <- read.csv("DF_SEED1234_0p1", header=TRUE)
#df_movies <- read.csv("DF_1p00", header=TRUE)
movies <- as(df_movies, "realRatingMatrix")

r <- sample(movies, 1000)


rowCounts(r[1,])
as(r[1,], "list")

rec <- Recommender(movies[1:1000], method = "POPULAR")
names(getModel(rec))    # Shows models from the recommender
getModel(rec)$topN      # Model as a topN list to store the popularity and other elements to recommend.

recom <- predict(rec, movies[1001:1002], n=5)

recom <- predict(rec, movies[1001:1002], type="ratings")
#rowMeans(r[1,])
#hist(getRatings(r), breaks=100)   # getRatings() extracts a vector 
# with all non-missing ratings from a rating matrix
#hist(getRatings(normalize(r)), breaks=100)   # getRatings() extracts a vector 
# with all non-missing ratings from a rating matrix
#hist(getRatings(normalize(r,method="Z-score")), breaks=100)   # this shows standard deviations
#hist(rowCounts(r), breaks=50)
#hist(colMeans(r), breaks=20)


#r <- Recommender(movies[1:100000], method = "POPULAR")
#
#recom <- predict(r, movies[100001:100002], n=5)
#as(recom, "matrix")[,1:10]     # Shows NA where that user gave a rating (prediction not needed)

