# SIGMOD 2023 - Programming Contest Solution

Team name: jfpk

[Leaderboard][1]

# Solution

Cluster points and then exhaustively search each point's cluster for its 100
nearest neighbors.

# Runtimes

Local machine is i7-1185G7 CPU (4 cores, 8 threads), 16GB RAM.\
Eval machine is Azure Standard F32s_v2 (32 cores, ? threads), 64GB RAM.

10.000 dataset refers to dummy-data.bin locally.
We cannot evaluate its recall on the eval machine.

1.000.000 dataset refers to contest-data-release-1m.bin locally.
We cannot evaluate its recall on the eval machine.

10.000.000 dataset refers to contest-data-release-10m.bin locally.
The eval machine evaluates a similar private dataset of equal cardinality.

| Dataset    | # Clusters | Recall | Runtime local | Runtime eval           |
|------------|------------|--------|---------------|------------------------|
| 10.000     | 1          | ?      | 1.750 secs    | Evaluation done on 10m |
| 10.000     | 2          | ?      | 1.250 secs    | Evaluation done on 10m |
| 10.000     | 3          | ?      | 0.950 secs    | Evaluation done on 10m |
| 10.000     | 4          | ?      | 0.800 secs    | Evaluation done on 10m |
| 10.000     | 5          | ?      | 0.610 secs    | Evaluation done on 10m |
| 10.000     | 6          | ?      | 0.650 secs    | Evaluation done on 10m |
| 10.000     | 7          | ?      | 0.630 secs    | Evaluation done on 10m |
| 10.000     | 8          | ?      | 0.700 secs    | Evaluation done on 10m |
| 10.000     | 9          | ?      | 0.630 secs    | Evaluation done on 10m |
| 10.000     | 10         | ?      | 0.500 secs    | Evaluation done on 10m |
| 10.000     | 11         | ?      | 0.760 secs    | Evaluation done on 10m |
| 10.000     | 12         | ?      | 0.550 secs    | Evaluation done on 10m |
| 10.000     | 13         | ?      | 0.475 secs    | Evaluation done on 10m |
| 10.000     | 14         | ?      | 0.420 secs    | Evaluation done on 10m |

# Ideas
Baseline uses euclidean distance comparison only version. This version does
not include a square root over the result. More [here][2].

Approximate clustering exists. Can we implement it? Does it offer better
tradeoff over recall vs construction time?

Plot the recall score and runtime for various number of clusters.

[1]: http://sigmod2023contest.eastus.cloudapp.azure.com/leaders_test.shtml
[2]: https://en.wikibooks.org/wiki/Algorithms/Distance_approximations
