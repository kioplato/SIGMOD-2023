# SIGMOD 2023 - Programming Contest Solution

Team name: jfpk

[Leaderboard][1]

# Solution

Cluster points and then exhaustively search each point's cluster for its 100
nearest neighbors.

# Runtimes

| Dataset                     | # Clusters | Recall | Runtime local (4 cores, 8 thr) | Runtime eval (32 cores, ? thr) |
|-----------------------------|------------|--------|--------------------------------|--------------------------------|
| dummy-data.bin              | 1          | ?      | 1.750 secs                     | Evaluation done on 10m         |
| dummy-data.bin              | 2          | ?      | 1.250 secs                     | Evaluation done on 10m         |
| dummy-data.bin              | 3          | ?      | 0.950 secs                     | Evaluation done on 10m         |
| dummy-data.bin              | 4          | ?      | 0.800 secs                     | Evaluation done on 10m         |
| dummy-data.bin              | 5          | ?      | 0.610 secs                     | Evaluation done on 10m         |
| dummy-data.bin              | 6          | ?      | 0.650 secs                     | Evaluation done on 10m         |
| dummy-data.bin              | 7          | ?      | 0.630 secs                     | Evaluation done on 10m         |
| dummy-data.bin              | 8          | ?      | 0.700 secs                     | Evaluation done on 10m         |
| dummy-data.bin              | 9          | ?      | 0.630 secs                     | Evaluation done on 10m         |
| dummy-data.bin              | 10         | ?      | 0.500 secs                     | Evaluation done on 10m         |
| dummy-data.bin              | 11         | ?      | 0.760 secs                     | Evaluation done on 10m         |
| dummy-data.bin              | 12         | ?      | 0.550 secs                     | Evaluation done on 10m         |
| dummy-data.bin              | 13         | ?      | 0.475 secs                     | Evaluation done on 10m         |
| dummy-data.bin              | 14         | ?      | 0.420 secs                     | Evaluation done on 10m         |

# Ideas
Baseline uses euclidean distance comparison only version. This version does
not include a square root over the result. More [here][2].

Approximate clustering exists. Can we implement it? Does it offer better
tradeoff over recall vs construction time?

Plot the recall score and runtime for various number of clusters.

[1]: http://sigmod2023contest.eastus.cloudapp.azure.com/leaders_test.shtml
[2]: https://en.wikibooks.org/wiki/Algorithms/Distance_approximations
