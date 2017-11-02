library("recommenderlab")
m <- matrix(sample(c(as.numeric(0:5), NA), 50, replace=TRUE, prob=c(rep(.4/6,6),.6)), ncol = 10,
            dimnames=list(user=paste("u", 1:5, sep=''), item=paste("i", 1:10, sep='')))
r <- as(m, "realRatingMatrix")
getRatingMatrix(r)