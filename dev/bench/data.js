window.BENCHMARK_DATA = {
  "lastUpdate": 1612666030237,
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
      },
      {
        "commit": {
          "author": {
            "email": "tkcandrade@gmail.com",
            "name": "Thamara Andrade",
            "username": "thamara"
          },
          "committer": {
            "email": "brenorg@gmail.com",
            "name": "Breno Rodrigues Guimarães",
            "username": "brenoguim"
          },
          "distinct": true,
          "id": "dba3df37dcf358feab76ca465a631693ba41f0f2",
          "message": "Using mdsnippets to generate shared_array_example in README",
          "timestamp": "2020-10-27T14:23:26-03:00",
          "tree_id": "50a2accebcd7af8cba201e6ee8ba04b9bcb13448",
          "url": "https://github.com/brenoguim/flexclass/commit/dba3df37dcf358feab76ca465a631693ba41f0f2"
        },
        "date": 1603819499104,
        "tool": "catch2",
        "benches": [
          {
            "name": "Sum all ids on nofc graph",
            "value": 14.6047,
            "range": "± 4.82585",
            "unit": "us",
            "extra": "100 samples\n3 iterations"
          },
          {
            "name": "Sum all ids on withfc graph",
            "value": 11.5248,
            "range": "± 2.9376",
            "unit": "us",
            "extra": "100 samples\n4 iterations"
          },
          {
            "name": "Sum all link ptrs on nofc graph",
            "value": 13.8806,
            "range": "± 3.61366",
            "unit": "us",
            "extra": "100 samples\n3 iterations"
          },
          {
            "name": "Sum all link ptrs on withfc graph",
            "value": 3.87706,
            "range": "± 706.781",
            "unit": "us",
            "extra": "100 samples\n9 iterations"
          },
          {
            "name": "Create DAG no fc",
            "value": 2.23704,
            "range": "± 282.682",
            "unit": "ms",
            "extra": "100 samples\n1 iterations"
          },
          {
            "name": "Create DAG with fc",
            "value": 1.80069,
            "range": "± 169.906",
            "unit": "ms",
            "extra": "100 samples\n1 iterations"
          },
          {
            "name": "Traverse DAG no fc",
            "value": 2.91678,
            "range": "± 180.405",
            "unit": "ms",
            "extra": "100 samples\n1 iterations"
          },
          {
            "name": "Traverse DAG with fc",
            "value": 2.58508,
            "range": "± 268.411",
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
          "id": "707b9651ffd1be38fb1843aeac0cc96762589605",
          "message": "Merge pull request #23 from eullerborges/master\n\nAdding preprocessed library size check (fixes #12)",
          "timestamp": "2020-10-27T14:26:19-03:00",
          "tree_id": "5b336bce57cb9bc346b569abbfbdb40d2f4b1908",
          "url": "https://github.com/brenoguim/flexclass/commit/707b9651ffd1be38fb1843aeac0cc96762589605"
        },
        "date": 1603819674785,
        "tool": "catch2",
        "benches": [
          {
            "name": "Sum all ids on nofc graph",
            "value": 28.1666,
            "range": "± 7.88464",
            "unit": "us",
            "extra": "100 samples\n2 iterations"
          },
          {
            "name": "Sum all ids on withfc graph",
            "value": 34.1295,
            "range": "± 20.2548",
            "unit": "us",
            "extra": "100 samples\n2 iterations"
          },
          {
            "name": "Sum all link ptrs on nofc graph",
            "value": 32.3951,
            "range": "± 8.33826",
            "unit": "us",
            "extra": "100 samples\n2 iterations"
          },
          {
            "name": "Sum all link ptrs on withfc graph",
            "value": 4.23984,
            "range": "± 559.87",
            "unit": "us",
            "extra": "100 samples\n9 iterations"
          },
          {
            "name": "Create DAG no fc",
            "value": 2.49,
            "range": "± 125.225",
            "unit": "ms",
            "extra": "100 samples\n1 iterations"
          },
          {
            "name": "Create DAG with fc",
            "value": 1.93327,
            "range": "± 67.1411",
            "unit": "ms",
            "extra": "100 samples\n1 iterations"
          },
          {
            "name": "Traverse DAG no fc",
            "value": 3.76204,
            "range": "± 242.978",
            "unit": "ms",
            "extra": "100 samples\n1 iterations"
          },
          {
            "name": "Traverse DAG with fc",
            "value": 3.39203,
            "range": "± 236.381",
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
          "id": "b586655975ec1e6a816e247d65a84dbb88d7e6af",
          "message": "Merge pull request #28 from brenoguim/fix_perf_tests\n\nFix perf tests",
          "timestamp": "2021-02-06T20:15:59-03:00",
          "tree_id": "1726856c15d8a45f22a559f97044d6ebb35ac839",
          "url": "https://github.com/brenoguim/flexclass/commit/b586655975ec1e6a816e247d65a84dbb88d7e6af"
        },
        "date": 1612653443134,
        "tool": "catch2",
        "benches": [
          {
            "name": "Sum all ids on nofc graph",
            "value": 12.891,
            "range": "± 1.62652",
            "unit": "us",
            "extra": "100 samples\n3 iterations"
          },
          {
            "name": "Sum all ids on withfc graph",
            "value": 12.1695,
            "range": "± 1.25024",
            "unit": "us",
            "extra": "100 samples\n4 iterations"
          },
          {
            "name": "Sum all link ptrs on nofc graph",
            "value": 13.466,
            "range": "± 1.51772",
            "unit": "us",
            "extra": "100 samples\n3 iterations"
          },
          {
            "name": "Sum all link ptrs on withfc graph",
            "value": 4.48624,
            "range": "± 322.868",
            "unit": "us",
            "extra": "100 samples\n9 iterations"
          },
          {
            "name": "Create DAG no fc",
            "value": 2.56785,
            "range": "± 25.1233",
            "unit": "ms",
            "extra": "100 samples\n1 iterations"
          },
          {
            "name": "Create DAG with fc",
            "value": 1.95946,
            "range": "± 34.2945",
            "unit": "ms",
            "extra": "100 samples\n1 iterations"
          },
          {
            "name": "Traverse DAG no fc",
            "value": 3.00431,
            "range": "± 66.5809",
            "unit": "ms",
            "extra": "100 samples\n1 iterations"
          },
          {
            "name": "Traverse DAG with fc",
            "value": 2.81261,
            "range": "± 36.9928",
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
          "id": "743f6abc0c5daa4662030239ecd9e6472aa0c768",
          "message": "Merge pull request #24 from eullerborges/coverage\n\nAdding codecov.io support",
          "timestamp": "2021-02-06T20:38:00-03:00",
          "tree_id": "a7c01cb6018faebb9d44a045acf4cb797991e391",
          "url": "https://github.com/brenoguim/flexclass/commit/743f6abc0c5daa4662030239ecd9e6472aa0c768"
        },
        "date": 1612654769323,
        "tool": "catch2",
        "benches": [
          {
            "name": "Sum all ids on nofc graph",
            "value": 13.7087,
            "range": "± 2.29774",
            "unit": "us",
            "extra": "100 samples\n3 iterations"
          },
          {
            "name": "Sum all ids on withfc graph",
            "value": 11.8408,
            "range": "± 2.07208",
            "unit": "us",
            "extra": "100 samples\n4 iterations"
          },
          {
            "name": "Sum all link ptrs on nofc graph",
            "value": 13.8747,
            "range": "± 1.65144",
            "unit": "us",
            "extra": "100 samples\n3 iterations"
          },
          {
            "name": "Sum all link ptrs on withfc graph",
            "value": 4.49012,
            "range": "± 76.4059",
            "unit": "us",
            "extra": "100 samples\n9 iterations"
          },
          {
            "name": "Create DAG no fc",
            "value": 2.52839,
            "range": "± 6.66902",
            "unit": "ms",
            "extra": "100 samples\n1 iterations"
          },
          {
            "name": "Create DAG with fc",
            "value": 1.92472,
            "range": "± 33.1447",
            "unit": "ms",
            "extra": "100 samples\n1 iterations"
          },
          {
            "name": "Traverse DAG no fc",
            "value": 3.00254,
            "range": "± 41.5208",
            "unit": "ms",
            "extra": "100 samples\n1 iterations"
          },
          {
            "name": "Traverse DAG with fc",
            "value": 2.80842,
            "range": "± 33.2379",
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
          "id": "ff6255be0f1e0f2f219878b0f98a0c537b6cfb68",
          "message": "Merge pull request #25 from eullerborges/cov_genhtml\n\nAdding HTML-generating coverage report script",
          "timestamp": "2021-02-06T20:38:43-03:00",
          "tree_id": "522f1f23c6881cc7958cc13982903d78916c5068",
          "url": "https://github.com/brenoguim/flexclass/commit/ff6255be0f1e0f2f219878b0f98a0c537b6cfb68"
        },
        "date": 1612654812491,
        "tool": "catch2",
        "benches": [
          {
            "name": "Sum all ids on nofc graph",
            "value": 13.5625,
            "range": "± 1.70014",
            "unit": "us",
            "extra": "100 samples\n3 iterations"
          },
          {
            "name": "Sum all ids on withfc graph",
            "value": 14.8135,
            "range": "± 1.39748",
            "unit": "us",
            "extra": "100 samples\n3 iterations"
          },
          {
            "name": "Sum all link ptrs on nofc graph",
            "value": 13.5682,
            "range": "± 2.02621",
            "unit": "us",
            "extra": "100 samples\n3 iterations"
          },
          {
            "name": "Sum all link ptrs on withfc graph",
            "value": 4.50009,
            "range": "± 77.029",
            "unit": "us",
            "extra": "100 samples\n9 iterations"
          },
          {
            "name": "Create DAG no fc",
            "value": 2.53009,
            "range": "± 6.85631",
            "unit": "ms",
            "extra": "100 samples\n1 iterations"
          },
          {
            "name": "Create DAG with fc",
            "value": 1.93526,
            "range": "± 5.96148",
            "unit": "ms",
            "extra": "100 samples\n1 iterations"
          },
          {
            "name": "Traverse DAG no fc",
            "value": 3.01648,
            "range": "± 41.6862",
            "unit": "ms",
            "extra": "100 samples\n1 iterations"
          },
          {
            "name": "Traverse DAG with fc",
            "value": 2.80543,
            "range": "± 33.4136",
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
          "id": "54e6c0094dfac65593633a7e4fd3c747210aec74",
          "message": "Adjust coverage configuration\n\nAdd token and use the correct workdir",
          "timestamp": "2021-02-06T22:32:41-03:00",
          "tree_id": "ca76ea9d666e22c539018e8a2e57d5ae2973dae3",
          "url": "https://github.com/brenoguim/flexclass/commit/54e6c0094dfac65593633a7e4fd3c747210aec74"
        },
        "date": 1612661637409,
        "tool": "catch2",
        "benches": [
          {
            "name": "Sum all ids on nofc graph",
            "value": 12.1583,
            "range": "± 2.65184",
            "unit": "us",
            "extra": "100 samples\n3 iterations"
          },
          {
            "name": "Sum all ids on withfc graph",
            "value": 10.5045,
            "range": "± 692.458",
            "unit": "us",
            "extra": "100 samples\n3 iterations"
          },
          {
            "name": "Sum all link ptrs on nofc graph",
            "value": 12.9877,
            "range": "± 1.78119",
            "unit": "us",
            "extra": "100 samples\n3 iterations"
          },
          {
            "name": "Sum all link ptrs on withfc graph",
            "value": 3.91685,
            "range": "± 551.722",
            "unit": "us",
            "extra": "100 samples\n9 iterations"
          },
          {
            "name": "Create DAG no fc",
            "value": 2.10034,
            "range": "± 245.509",
            "unit": "ms",
            "extra": "100 samples\n1 iterations"
          },
          {
            "name": "Create DAG with fc",
            "value": 1.52209,
            "range": "± 170.985",
            "unit": "ms",
            "extra": "100 samples\n1 iterations"
          },
          {
            "name": "Traverse DAG no fc",
            "value": 2.63408,
            "range": "± 270.621",
            "unit": "ms",
            "extra": "100 samples\n1 iterations"
          },
          {
            "name": "Traverse DAG with fc",
            "value": 2.39521,
            "range": "± 248.404",
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
          "id": "82ff3d047022c87c693b29309ccf5706580600d2",
          "message": "Adjust coverage configuration\n\n* Copy codecov.yml to build folder otherwise codecov doesn't find it. Weird.\r\n* Remap names so files are found. Might be doing something wrong there, but now it works.",
          "timestamp": "2021-02-06T23:04:32-03:00",
          "tree_id": "af4c9aebe04f8525ebac32511a22d49900aaf62f",
          "url": "https://github.com/brenoguim/flexclass/commit/82ff3d047022c87c693b29309ccf5706580600d2"
        },
        "date": 1612663557782,
        "tool": "catch2",
        "benches": [
          {
            "name": "Sum all ids on nofc graph",
            "value": 29.2297,
            "range": "± 6.63817",
            "unit": "us",
            "extra": "100 samples\n2 iterations"
          },
          {
            "name": "Sum all ids on withfc graph",
            "value": 31.1608,
            "range": "± 9.43852",
            "unit": "us",
            "extra": "100 samples\n2 iterations"
          },
          {
            "name": "Sum all link ptrs on nofc graph",
            "value": 27.9522,
            "range": "± 5.86554",
            "unit": "us",
            "extra": "100 samples\n2 iterations"
          },
          {
            "name": "Sum all link ptrs on withfc graph",
            "value": 4.41337,
            "range": "± 1.6356",
            "unit": "us",
            "extra": "100 samples\n9 iterations"
          },
          {
            "name": "Create DAG no fc",
            "value": 2.34786,
            "range": "± 145.503",
            "unit": "ms",
            "extra": "100 samples\n1 iterations"
          },
          {
            "name": "Create DAG with fc",
            "value": 1.96317,
            "range": "± 146.025",
            "unit": "ms",
            "extra": "100 samples\n1 iterations"
          },
          {
            "name": "Traverse DAG no fc",
            "value": 3.74145,
            "range": "± 283.77",
            "unit": "ms",
            "extra": "100 samples\n1 iterations"
          },
          {
            "name": "Traverse DAG with fc",
            "value": 3.17787,
            "range": "± 152.178",
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
          "id": "41a012d266009be7b4c48c12f4e5621e99f3d6df",
          "message": "Start documenting interaction with other features",
          "timestamp": "2021-02-06T23:45:55-03:00",
          "tree_id": "8b831496a1eaa058a481d828316ed04b798c90e3",
          "url": "https://github.com/brenoguim/flexclass/commit/41a012d266009be7b4c48c12f4e5621e99f3d6df"
        },
        "date": 1612666029309,
        "tool": "catch2",
        "benches": [
          {
            "name": "Sum all ids on nofc graph",
            "value": 26.1977,
            "range": "± 5.12215",
            "unit": "us",
            "extra": "100 samples\n2 iterations"
          },
          {
            "name": "Sum all ids on withfc graph",
            "value": 26.5607,
            "range": "± 4.76737",
            "unit": "us",
            "extra": "100 samples\n2 iterations"
          },
          {
            "name": "Sum all link ptrs on nofc graph",
            "value": 29.0192,
            "range": "± 11.3359",
            "unit": "us",
            "extra": "100 samples\n1 iterations"
          },
          {
            "name": "Sum all link ptrs on withfc graph",
            "value": 3.20136,
            "range": "± 597.832",
            "unit": "us",
            "extra": "100 samples\n9 iterations"
          },
          {
            "name": "Create DAG no fc",
            "value": 1.95731,
            "range": "± 306.875",
            "unit": "ms",
            "extra": "100 samples\n1 iterations"
          },
          {
            "name": "Create DAG with fc",
            "value": 1.43274,
            "range": "± 133.93",
            "unit": "ms",
            "extra": "100 samples\n1 iterations"
          },
          {
            "name": "Traverse DAG no fc",
            "value": 3.12713,
            "range": "± 254.409",
            "unit": "ms",
            "extra": "100 samples\n1 iterations"
          },
          {
            "name": "Traverse DAG with fc",
            "value": 2.74285,
            "range": "± 190.841",
            "unit": "ms",
            "extra": "100 samples\n1 iterations"
          }
        ]
      }
    ]
  }
}