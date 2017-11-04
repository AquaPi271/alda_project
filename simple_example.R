library("recommenderlab")
m <- matrix(sample(c(as.numeric(0:5), NA), 50, replace=TRUE, prob=c(rep(.4/6,6),.6)), ncol = 10,
            dimnames=list(user=paste("u", 1:5, sep=''), item=paste("i", 1:10, sep='')))
r <- as(m, "realRatingMatrix")
# getRatingMatrix(r)         # Display the matrix
# head(as(r,"data.frame"))   # See as data frame which can be read or written (csv, etc.)
r_m <- normalize(r)          # Center mean / variance
# getRatingMatrix(r_m)       # Display the matrix
denormalize(r_m)             # Return to original matrix
image(r, main = "Raw Ratings")  # Generates image of subset
image(r_m, main = "Normalized Ratings")  # Generates image of subset
r_b <- binarize(r, minRating=4 )   # Convert to 0 or 1 with minRating the threshold for 1
as(r_b, "matrix")