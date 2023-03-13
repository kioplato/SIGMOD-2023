# SIGMOD 2023 - Programming Contest Solution

Team name: jfpk

[Leaderboard][1]

# Solution

Cluster points and then exhaustively search each point's cluster for its 100
nearest neighbors.

# Runtimes

Local machine is i7-1185G7 CPU (4 cores, 8 threads), 16GB RAM.\
Eval machine is Azure Standard F32s_v2 (32 cores, ? threads), 64GB RAM.

| Dataset | # Clusters | Recall | Runtime local | Runtime eval |
|---------|------------|--------|---------------|--------------|
| 10k     | 1          | ?      | 1.750 secs    | NaN          |
| 10k     | 2          | ?      | 1.250 secs    | NaN          |
| 10k     | 3          | ?      | 0.950 secs    | NaN          |
| 10k     | 4          | ?      | 0.800 secs    | NaN          |
| 10k     | 5          | ?      | 0.610 secs    | NaN          |
| 10k     | 6          | ?      | 0.650 secs    | NaN          |
| 10k     | 7          | ?      | 0.630 secs    | NaN          |
| 10k     | 8          | ?      | 0.700 secs    | NaN          |
| 10k     | 9          | ?      | 0.630 secs    | NaN          |
| 10k     | 10         | ?      | 0.500 secs    | NaN          |
| 10k     | 11         | ?      | 0.760 secs    | NaN          |
| 10k     | 12         | ?      | 0.550 secs    | NaN          |
| 10k     | 13         | ?      | 0.475 secs    | NaN          |
| 10k     | 14         | ?      | 0.420 secs    | NaN          |
| 10m     | 8          | NaN    | ?             | TLE          |
| 10m     | 32         | NaN    | ?             | TLE          |

# Ideas
Baseline uses euclidean distance comparison only version. This version does
not include a square root over the result. More [here][2].

Approximate clustering exists. Can we implement it? Does it offer better
tradeoff over recall vs construction time?

Plot the recall score and runtime for various number of clusters.

# Find best hyperparameters

Inside scripts/ directory the script `benchmark-hyperparameters.sh` benchmarks
all hyperparameters in the specified ranges. It creates all the combinations
of their values and benchmarks all combinations. The benchmarks are written in
the specified file as a markdown table. Each benchmark, a row in the table,
includes the execution time required and its recall score.

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
