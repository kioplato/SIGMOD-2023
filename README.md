# SIGMOD 2023 - Programming Contest Solution

Team name: jfpk

[Leaderboard][1]

# Solutions

Four different solutions have been developed.

* Samples: take a random subset of specified cardinality and exhaustively search
within that sample only. This solution is based on the baseline solution
provided by the organizers.

* Nearest clusters: perform clustering and then exhaustively search in the
nearest clusters. The number of clusters to create and the number of nearest
clusters to search can be specified from cli arguments.

* kgraph-index: kgraph algorithm creates a knng to answer queries for nearest
neighbors of a specified point.

* nn-join: based on Near Neighbor Join paper.

# Runtimes

The benchmarks are performed on multiple computers:
* Local: i7 1185G7, 16GB RAM.
* Vultr 8: 8 dedicated vcpus, 16GB RAM.
* Eval: Azure Standard F32s\_v2 (32 cores, 32 threads), 64GB RAM.

## Samples solution benchmarks

| Dataset | Sample size | Local time | Vultr 8 time | Eval time | Ratio Vultr/Eval | Recall |
|---------|-------------|------------|--------------|-----------|------------------|--------|
| 10m     | 100         | 38         | 31           | 91        | 0.340659         | 0.000  |
| 10m     | 1000        | 240        | 267          | 140       | 1.907143         | 0.000  |
| 10m     | 5000        | 1352       | 1242         | 400       | 3.105            | 0.000  |
| 10m     | 10000       |            | 2344         | 727       | 3.224209         | 0.001  |
| 10m     | 12500       |            | 3472         | 953       | 3.643232         | 0.001  |
| 10m     | 15000       |            |              | 1038      |                  | 0.001  |
| 10m     | 20000       |            |              | 1355      |                  | 0.002  |
| 10m     | 30000       |            |              | >1860     |                  |        |

## Nearest clusters solution benchmarks

| Dataset | # Clusters | # Iterations | # Nearest clusters | Local time | Eval time | Recall |
|---------|------------|--------------|--------------------|------------|-----------|--------|
| 10m     | 500\_000   | 1            | 15                 | TBD        | TLE       | TBD    |
| 10m     | 50\_000    | 1            | 1                  | TBD        | TLE       | TBD    |
| 10m     | 10\_000    | 1            | 1                  | TBD        | 1493      | 0.067  |
| 10m     | 12_500     | 1            | 2                  | TBD        | TLE       | TBD    |
| 10m     | 7_500      | 1            | 1                  | TBD        | 1308      | 0.071  |
| 10m     | 7_500      | 1            | 2                  | TBD        | 1699      | 0.116  |
| 10m     | 3_750      | 1            | 1                  | TBD        | 1224      | 0.086  |
| 10m     | 3_750      | 1            | 2                  | TBD        |           |        |

## KGraph index construction

## Near neighbor join

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
