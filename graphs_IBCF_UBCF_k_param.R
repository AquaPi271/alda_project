library("reshape2")
library("ggplot2")
library("gridExtra")

cf_k_table <- read.delim(text="k UBCF_COSINE_DENORMALIZED IBCF_COSINE_DENORMALIZED UBCF_COSINE_NORMALIZED IBCF_COSINE_NORMALIZED UBCF_PEARSON_DENORMALIZED IBCF_PEARSON_DENORMALIZED UBCF_PEARSON_NORMALIZED IBCF_PEARSON_NORMALIZED
1 1.23655 0.920048 1.11839 0.920048 1.23655 0.920048 1.11839 0.920048
2 1.10926 1.00381 0.888671 0.905941 0.997885 0.878653 0.98337 0.881869
3 1.10323 0.987016 0.850719 0.864337 0.953396 0.83077 0.89544 0.833321
4 1.0765 0.987237 0.817349 0.839805 0.913284 0.810032 0.843541 0.806225
5 1.04544 0.981972 0.805295 0.821832 0.897593 0.799235 0.829648 0.788883
6 1.03094 0.978671 0.786088 0.812109 0.875153 0.797812 0.822661 0.777133
7 1.01337 0.97742 0.779532 0.808517 0.86764 0.790032 0.804924 0.777174
8 1.00257 0.981971 0.775688 0.80096 0.870702 0.785709 0.798699 0.774489
9 1.00548 0.978952 0.773115 0.798931 0.86799 0.779873 0.793205 0.767587
10 1.00712 0.980134 0.774617 0.795652 0.864337 0.776034 0.791223 0.759063
11 1.01112 0.979088 0.771025 0.792216 0.865305 0.772696 0.783226 0.756642
12 1.00416 0.977 0.772224 0.787178 0.868169 0.771713 0.775699 0.758621
13 1.00034 0.977806 0.771314 0.788605 0.863967 0.771328 0.772654 0.757013
14 0.997866 0.977861 0.769123 0.788509 0.862327 0.767712 0.776436 0.757467
15 0.994801 0.976343 0.766873 0.787973 0.864196 0.769112 0.779886 0.757156
16 0.991728 0.97521 0.764871 0.78885 0.862905 0.769519 0.777933 0.759028
17 0.988238 0.976085 0.763372 0.78805 0.861703 0.771228 0.775126 0.759139
",as.is=TRUE,sep=" ",header=TRUE)

#mdf <- melt(cf_k_table, id="k", measure.name="UBCF_COSINE_DENORMALIZED", variable.name="UBCF_COSINE_DENORMALIZED")
#ggplot(data=mdf,
#       aes(x=k, y=UBCF_COSINE_DENORMALIZED)) +
#  geom_line()

p1 <- ggplot(cf_k_table, aes(k)) +
  geom_point(aes(y=UBCF_PEARSON_NORMALIZED, colour = "UBCF")) +
  geom_line(aes(y=UBCF_PEARSON_NORMALIZED, colour = "UBCF"), size=1) +
  geom_point(aes(y=IBCF_PEARSON_NORMALIZED, colour = "IBCF")) +
  geom_line(aes(y=IBCF_PEARSON_NORMALIZED, colour = "IBCF"), size=1) +
  labs(y = "RMSE") +
  theme(panel.background = element_blank(), legend.position = "none")

  p2 <- ggplot(cf_k_table, aes(k)) +
  geom_point(aes(y=UBCF_COSINE_NORMALIZED, colour = "UBCF")) +
  geom_line(aes(y=UBCF_COSINE_NORMALIZED, colour = "UBCF"), size=1) +
  geom_point(aes(y=IBCF_COSINE_NORMALIZED, colour = "IBCF")) +
  geom_line(aes(y=IBCF_COSINE_NORMALIZED, colour = "IBCF"), size=1) +
  labs(y = "RMSE") +
  theme(panel.background = element_blank(), legend.position = "none")
#  grid.arrange(p1,p2,ncol=2)
  
  cf_ubcf_compare_table <- read.delim(text="Measurement,Base_Line,Cosine_Denorm,Cosine_Norm,Pearson_Denorm,Pearson_Norm,
1,1.0797,1.19458,0.886032,0.958599,0.93466,
2,1.11195,1.18205,0.831459,1.01042,0.86987,
3,1.07182,1.18017,0.825513,0.974755,0.873476,
4,1.03431,1.16384,0.815974,0.903752,0.858213,
5,1.02696,1.13505,0.861247,0.970767,0.886639,
6,1.03228,1.13174,0.855554,0.967849,0.894999,
7,1.03952,1.11968,0.849884,0.966142,0.88241,
8,1.02817,1.10991,0.847066,0.965589,0.881449,
9,1.01519,1.10254,0.843545,0.944243,0.902373,
10,1.03265,1.10207,0.836399,0.933183,0.867191,
11,1.03867,1.10052,0.856144,0.977383,0.891405,
12,1.02151,1.09749,0.854037,0.975044,0.890092,
13,1.05139,1.09585,0.875231,0.989457,0.918303,
14,1.04383,1.08867,0.848398,0.994004,0.890411,
15,1.00432,1.07522,0.822458,0.923591,0.856352,
16,1.00754,1.06909,0.86171,0.956693,0.88929,
17,0.991315,1.05956,0.848119,0.942754,0.881036,
18,0.99768,1.05817,0.839758,0.956986,0.870892,
19,0.97684,1.04926,0.826053,0.942877,0.852991,
20,0.989388,1.00991,0.829869,0.947265,0.865501,
21,0.936467,1.00712,0.766873,0.864337,0.779886,
22,0.936927,1.0034,0.831235,0.897187,0.869117,
23,0.972416,0.998826,0.829542,0.920469,0.857756,
24,0.969563,0.97859,0.828095,0.920952,0.86166,
25,0.968466,0.971134,0.816331,0.917372,0.858401
",as.is=TRUE,sep=",",header=TRUE)

p1 <- ggplot(cf_ubcf_compare_table, aes(Measurement)) +
  geom_point(aes(y=Base_Line, colour = "Base Line")) +
  geom_line(aes(y=Base_Line, colour = "Base Line"), size=1) +
  geom_point(aes(y=Cosine_Denorm, colour = "Cosine Denorm")) +
  geom_line(aes(y=Cosine_Denorm, colour = "Cosine Denorm"), size=1) +
  geom_point(aes(y=Cosine_Norm, colour = "Cosine Norm")) +
  geom_line(aes(y=Cosine_Norm, colour = "Cosine Norm"), size=1) +
  geom_point(aes(y=Pearson_Denorm, colour = "Pearson Denorm")) +
  geom_line(aes(y=Pearson_Denorm, colour = "Pearson Denorm"), size=1) +
  geom_point(aes(y=Pearson_Norm, colour = "Pearson Norm")) +
  geom_line(aes(y=Pearson_Norm, colour = "Pearson Norm"), size=1) +
  labs(y = "RMSE") +
  theme(panel.background = element_blank())


grid.arrange(p1,padding=10,ncol=1)