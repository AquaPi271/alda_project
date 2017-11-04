library(recommenderlab)
data(Jester5k)
set.seed(1234)
r <- sample(Jester5k, 1000)
rowCounts(r[1,])
as(r[1,], "list")
rowMeans(r[1,])
hist(getRatings(r), breaks=100)   # getRatings() extracts a vector 
# with all non-missing ratings from a rating matrix
hist(getRatings(normalize(r)), breaks=100)   # getRatings() extracts a vector 
# with all non-missing ratings from a rating matrix
hist(getRatings(normalize(r,method="Z-score")), breaks=100)   # this shows standard deviations
hist(rowCounts(r), breaks=50)
hist(colMeans(r), breaks=20)
