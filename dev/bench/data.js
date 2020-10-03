window.BENCHMARK_DATA = {
  "lastUpdate": 1601754971808,
  "repoUrl": "https://github.com/brenoguim/flexclass",
  "entries": {
    "Catch2 Benchmark": [
      {
        "commit": {
          "author": {
            "email": "eullerborges@opacium.com",
            "name": "Euller Borges",
            "username": "eullerborges"
          },
          "committer": {
            "email": "brenorg@gmail.com",
            "name": "Breno Rodrigues Guimarães",
            "username": "brenoguim"
          },
          "distinct": true,
          "id": "825c83717ba6e06a120f9513719dfa5599723bbc",
          "message": "Implementing benchmark regression\n\n- This implements a GitHub Action to track peformance benchmarks based\non https://github.com/rhysd/github-action-benchmark.",
          "timestamp": "2020-10-03T16:44:07-03:00",
          "tree_id": "1cc74fc2cd7ac10b3d02ffd5bd6e7d76b998a802",
          "url": "https://github.com/brenoguim/flexclass/commit/825c83717ba6e06a120f9513719dfa5599723bbc"
        },
        "date": 1601754338410,
        "tool": "catch2",
        "benches": [
          {
            "name": "Sum all ids on nofc graph",
            "value": 29.4931,
            "range": "± 6.78336",
            "unit": "us",
            "extra": "100 samples\n2 iterations"
          },
          {
            "name": "Sum all ids on withfc graph",
            "value": 31.2536,
            "range": "± 7.82999",
            "unit": "us",
            "extra": "100 samples\n2 iterations"
          },
          {
            "name": "Sum all link ptrs on nofc graph",
            "value": 28.3821,
            "range": "± 6.78456",
            "unit": "us",
            "extra": "100 samples\n2 iterations"
          },
          {
            "name": "Sum all link ptrs on withfc graph",
            "value": 4.17924,
            "range": "± 754.932",
            "unit": "us",
            "extra": "100 samples\n9 iterations"
          },
          {
            "name": "Create DAG no fc",
            "value": 2.39667,
            "range": "± 123.401",
            "unit": "ms",
            "extra": "100 samples\n1 iterations"
          },
          {
            "name": "Create DAG with fc",
            "value": 1.79202,
            "range": "± 138.13",
            "unit": "ms",
            "extra": "100 samples\n1 iterations"
          },
          {
            "name": "Traverse DAG no fc",
            "value": 3.71455,
            "range": "± 225.106",
            "unit": "ms",
            "extra": "100 samples\n1 iterations"
          },
          {
            "name": "Traverse DAG with fc",
            "value": 3.2408,
            "range": "± 215.361",
            "unit": "ms",
            "extra": "100 samples\n1 iterations"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "email": "brenorg@gmail.com",
            "name": "Breno Rodrigues Guimarães",
            "username": "brenoguim"
          },
          "committer": {
            "email": "noreply@github.com",
            "name": "GitHub",
            "username": "web-flow"
          },
          "distinct": true,
          "id": "6c66593781e77604d3cd696a3aa50b1a8722b260",
          "message": "Allow manual dispatch of regression tracking action",
          "timestamp": "2020-10-03T16:54:55-03:00",
          "tree_id": "5e5f3dfb7622652c0e013577ad185abb75d58812",
          "url": "https://github.com/brenoguim/flexclass/commit/6c66593781e77604d3cd696a3aa50b1a8722b260"
        },
        "date": 1601754971356,
        "tool": "catch2",
        "benches": [
          {
            "name": "Sum all ids on nofc graph",
            "value": 13.0195,
            "range": "± 1.27423",
            "unit": "us",
            "extra": "100 samples\n3 iterations"
          },
          {
            "name": "Sum all ids on withfc graph",
            "value": 10.4251,
            "range": "± 1.57337",
            "unit": "us",
            "extra": "100 samples\n3 iterations"
          },
          {
            "name": "Sum all link ptrs on nofc graph",
            "value": 9.75814,
            "range": "± 644.558",
            "unit": "us",
            "extra": "100 samples\n3 iterations"
          },
          {
            "name": "Sum all link ptrs on withfc graph",
            "value": 3.11579,
            "range": "± 41.2194",
            "unit": "us",
            "extra": "100 samples\n8 iterations"
          },
          {
            "name": "Create DAG no fc",
            "value": 1.90846,
            "range": "± 213.844",
            "unit": "ms",
            "extra": "100 samples\n1 iterations"
          },
          {
            "name": "Create DAG with fc",
            "value": 1.63066,
            "range": "± 183.086",
            "unit": "ms",
            "extra": "100 samples\n1 iterations"
          },
          {
            "name": "Traverse DAG no fc",
            "value": 2.4133,
            "range": "± 252.299",
            "unit": "ms",
            "extra": "100 samples\n1 iterations"
          },
          {
            "name": "Traverse DAG with fc",
            "value": 2.17011,
            "range": "± 234.876",
            "unit": "ms",
            "extra": "100 samples\n1 iterations"
          }
        ]
      }
    ]
  }
}