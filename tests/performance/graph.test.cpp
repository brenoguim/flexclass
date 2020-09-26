#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch.hpp>
#include <flexclass.hpp>

#include <memory>
#include <vector>

namespace nofc
{
    struct Node
    {
        std::size_t id;
        std::size_t numLinks;
        bool visited;
        std::unique_ptr<Node*[]> links;

        static auto make_unique(std::size_t id, std::size_t numLinks, bool visited, std::size_t size)
        {
            return std::unique_ptr<Node>(new Node{id, numLinks, visited, std::make_unique<Node*[]>(size)});
        }
    };

    auto& getVisited(Node* n) { return n->visited; }
    auto  getNumLinks(Node* n) { return n->numLinks; }
    auto  getLinks(Node* n) { return n->links.get(); }

    struct Dag
    {
        using N = Node;
        std::vector<std::unique_ptr<Node>> nodes;
    };

}

namespace withfc
{
    struct Node
    {
        auto fc_handles() { return fc::make_tuple(&links); }

        std::size_t id;
        std::size_t numLinks;
        bool visited;
        fc::AdjacentArray<Node*> links;

        static auto make_unique(std::size_t id, std::size_t numLinks, bool visited, std::size_t size)
        {
            return fc::make_unique<Node>(size)(id, numLinks, visited);
        }
    };

    auto& getVisited(Node* n) { return n->visited; }
    auto  getNumLinks(Node* n) { return n->numLinks; }
    auto  getLinks(Node* n) { return n->links.begin(n); }

    struct Dag
    {
        using N = Node;
        std::vector<fc::UniquePtr<Node>> nodes;
    };
}

namespace {
    template<class Dag>
    Dag makeRandomDag(std::size_t numNodes, int* inRands)
    {
        using Node = typename Dag::N;
        Dag g;
        g.nodes.reserve(numNodes);

        for (std::size_t i = 0; i < numNodes; ++i)
        {
            auto rands = inRands;

            auto gSize = g.nodes.size();

            std::size_t numLinks = gSize ? (((*rands++) % gSize % 20) + 1) : 0;

            auto n = Node::make_unique(g.nodes.size(), numLinks, false, numLinks);

            auto b = getLinks(n.get());
            for (std::size_t l = 0; l < numLinks; ++l)
                b[l] = g.nodes[(*rands++) % gSize].get();

            // Force a connection with the last node
            if (numLinks)
                b[numLinks-1] = g.nodes.back().get();

            g.nodes.push_back(std::move(n));
        }

        return g;
    }

    template<class Dag, class Fn>
    void traverseDag(Dag& g, Fn&& fn)
    {
        std::vector toProcess {g.nodes.back().get()};
        toProcess.reserve(g.nodes.size());

        while (!toProcess.empty())
        {
            auto n = toProcess.back();
            toProcess.pop_back();
            fn(n);

            getVisited(n) = true;

            auto b = getLinks(n);
            auto e = b + getNumLinks(n);

            for (; b != e; ++b)
                if (!getVisited(*b))
                    toProcess.push_back(*b);
        }

        for (auto& n : g.nodes) getVisited(n.get()) = false;
    }
}

TEST_CASE( "Test basic operations)", "[dag]")
{
    static constexpr std::size_t dagSize = 10000;

    srand(0);
    std::vector<int> randomNumbers;
    for (int i = 0; i < dagSize+1; ++i)
        randomNumbers.push_back(rand());

    auto nofcDag = makeRandomDag<nofc::Dag>(dagSize, &randomNumbers.front());
    auto withfcDag = makeRandomDag<withfc::Dag>(dagSize, &randomNumbers.front());

    BENCHMARK("Sum all ids on nofc graph") {
        std::size_t sum = 0;
        for (auto& n : nofcDag.nodes) sum += n->id;
        return sum;
    };

    BENCHMARK("Sum all ids on withfc graph") {
        std::size_t sum = 0;
        for (auto& n : withfcDag.nodes) sum += n->id;
        return sum;
    };

    BENCHMARK("Sum all link ptrs on nofc graph") {
        std::size_t sum = 0;
        for (auto& n : nofcDag.nodes) sum += (std::uintptr_t) getLinks(n.get());
        return sum;
    };

    BENCHMARK("Sum all link ptrs on withfc graph") {
        std::size_t sum = 0;
        for (auto& n : withfcDag.nodes) sum += (std::uintptr_t) getLinks(n.get());
        return sum;
    };
}

TEST_CASE( "Create a large random DAG (Directed Acyclic Graph)", "[dag]")
{
    static constexpr std::size_t dagSize = 10000;

    srand(0);
    std::vector<int> randomNumbers;
    for (int i = 0; i < dagSize+1; ++i)
        randomNumbers.push_back(rand());

    // Warm up
    { makeRandomDag<nofc::Dag>(dagSize, &randomNumbers.front()); }
    { makeRandomDag<withfc::Dag>(dagSize, &randomNumbers.front()); }

    BENCHMARK("Create DAG no fc") {
        return makeRandomDag<nofc::Dag>(dagSize, &randomNumbers.front());
    };

    BENCHMARK("Create DAG with fc") {
        return makeRandomDag<withfc::Dag>(dagSize, &randomNumbers.front());
    };

    auto nofcDag = makeRandomDag<nofc::Dag>(dagSize, &randomNumbers.front());
    auto withfcDag = makeRandomDag<withfc::Dag>(dagSize, &randomNumbers.front());

    BENCHMARK("Traverse DAG no fc") {
        int cnt = 0;
        traverseDag(nofcDag, [&cnt] (auto) { cnt++; });
        return cnt;
    };

    BENCHMARK("Traverse DAG with fc") {
        int cnt = 0;
        traverseDag(withfcDag, [&cnt] (auto) { cnt++; });
        return cnt;
    };
}
