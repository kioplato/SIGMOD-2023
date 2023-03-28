# SIGMOD 2023 - Programming Contest Solution

Team name: jfpk

[Leaderboard][1]

# Solutions

Four different solutions have been developed.

* Samples: take a random subset of specified cardinality and exhaustively search
within that sample only. This solution is based on the baseline solution
provided by the organizers.

* Nearest clusters: perform clustering and then exhaustively search in the
nearest clusters. The number of clusters to create an the number of nearest
clusters to search can be specified from cli arguments.

* kgraph-index: kgraph algorithm creates a knng to answer queries for nearest
neighbors of a specified point.

* nn-join: based on Near Neighbor Join paper.

# Runtimes

The benchmarks are performed on the local machine.\
Local machine is AMD EPYC 16 vcpu cores and 32GB RAM.

The evaluation is performed on the eval machine.\
Eval machine is Azure Standard F32s_v2 (32 cores, 32 threads), 64GB RAM.

The local machine is x2 slower than eval machine.\
Baseline solution for 1m dataset requires 20min on eval and 37min on local.

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
