# SIGMOD 2023 - Programming Contest Solution

Team name: jfpk

[Leaderboard][1]

# Solution

Cluster points and then exhaustively search each point's cluster for its 100
nearest neighbors.

# Runtimes

Local machine is i7-1185G7 CPU (4 cores, 8 threads), 16GB RAM.\
Eval machine is Azure Standard F32s_v2 (32 cores, ? threads), 64GB RAM.

| Dataset    | # Clusters | Recall | Runtime local | Runtime eval           |
|------------|------------|--------|---------------|------------------------|
| dummy-data | 1          | ?      | 1.750 secs    | Evaluation done on 10m |
| dummy-data | 2          | ?      | 1.250 secs    | Evaluation done on 10m |
| dummy-data | 3          | ?      | 0.950 secs    | Evaluation done on 10m |
| dummy-data | 4          | ?      | 0.800 secs    | Evaluation done on 10m |
| dummy-data | 5          | ?      | 0.610 secs    | Evaluation done on 10m |
| dummy-data | 6          | ?      | 0.650 secs    | Evaluation done on 10m |
| dummy-data | 7          | ?      | 0.630 secs    | Evaluation done on 10m |
| dummy-data | 8          | ?      | 0.700 secs    | Evaluation done on 10m |
| dummy-data | 9          | ?      | 0.630 secs    | Evaluation done on 10m |
| dummy-data | 10         | ?      | 0.500 secs    | Evaluation done on 10m |
| dummy-data | 11         | ?      | 0.760 secs    | Evaluation done on 10m |
| dummy-data | 12         | ?      | 0.550 secs    | Evaluation done on 10m |
| dummy-data | 13         | ?      | 0.475 secs    | Evaluation done on 10m |
| dummy-data | 14         | ?      | 0.420 secs    | Evaluation done on 10m |

# Ideas
Baseline uses euclidean distance comparison only version. This version does
not include a square root over the result. More [here][2].

Approximate clustering exists. Can we implement it? Does it offer better
tradeoff over recall vs construction time?

Plot the recall score and runtime for various number of clusters.

# Evaluator: How to evaluate a solution's recall

In evaluator/ directory two programs exist. The groundtruther and evaluator.

## Compute groundtruth

Using the `groundtruther` you can generate the true knng of a dataset.
You can also generate a true knng sample, which could be useful for faster
evaluation of the recall score of a aprox knng we would like to evaluate.

## Compute recall

Using the `recaller` you can compute the recall score of an aprox knng.
You should already have the complete true knng, or a sample of it, to provide
the recaller with in order to compute the recall score. The bigger the sample
of the true knng the more accurate will the recall score be of the aprox knng.

[1]: http://sigmod2023contest.eastus.cloudapp.azure.com/leaders_test.shtml
[2]: https://en.wikibooks.org/wiki/Algorithms/Distance_approximations
