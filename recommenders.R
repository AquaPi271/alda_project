# Recommenders
library(recommenderlab)
data(Jester5k)
set.seed(1234)

# Show all registered recommender functions:

recommenderRegistry$get_entries(dataType="realRatingMatrix")

# Create a recommender based on the number of users who have the item in their profile.

r <- Recommender(Jester5k[1:1000], method = "POPULAR")
names(getModel(r))    # Shows models from the recommender
getModel(r)$topN      # Model as a topN list to store the popularity and other elements to recommend.
recom <- predict(r, Jester5k[1001:1002], n=5)  # Pick top 5 items for users 1000 and 1001.
as(recom, "list")     
recom3 <- bestN(recom, n = 3)   # Returns top 3 elements 
as(recom3, "list")

# To predict ratings

recom <- predict(r, Jester5k[1001:1002], type="ratings")
as(recom, "matrix")[,1:10]     # Shows NA where that user gave a rating (prediction not needed)

recom <- predict(r, Jester5k[1001:1002], type="ratingMatrix")
as(recom, "matrix")[,1:10]     # Shows the full matrix, including the given user's rating
