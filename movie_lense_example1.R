library("recommenderlab")
data("MovieLense")
### use only users with more than 100 ratings
MovieLense100 <- MovieLense[rowCounts(MovieLense) >100,]
MovieLense100
train <- MovieLense100[1:50]
### learn user-based collaborative filtering recommender
rec <- Recommender(train, method = "UBCF")
rec
### create top-N recommendations for new users (users 101 and 102)
pre <- predict(rec, MovieLense100[101:102], n = 10)
pre
as(pre, "list")