# SIGMOD 2023 - Programming Contest Solution

Team name: jfpk

[Leaderboard][1]

# Solution

Cluster points and then exhaustively search each point's cluster for its 100
nearest neighbors.

# Runtimes

| Dataset        | # Clusters | Runtime local (secs) | Runtime eval (secs)    |
|----------------|------------|----------------------|------------------------|
| dummy-data.bin | 2          | 1.250 secs           | Evaluation done on 10m |

# Ideas
Baseline uses euclidean distance comparison only version. This version does
not include a square root over the result. More [here][2].

Approximate clustering exists. Can we implement it? Does it offer better
tradeoff over recall vs construction time?

Plot the recall score and runtime for various number of clusters.

[1]: http://sigmod2023contest.eastus.cloudapp.azure.com/leaders_test.shtml
[2]: https://en.wikibooks.org/wiki/Algorithms/Distance_approximations
