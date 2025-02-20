#include <iostream>
#include <vector>
#include <tuple>
#include <algorithm>  
#include <chrono>
#include "graph.h"
#include "skiplist.h" 

bool isNodeAligned(int node, const std::vector<int>& seedNodes)
{
    // Linear check. For large data sets, consider a set or unordered_set for O(log n) / O(1) lookups.
    return (std::find(seedNodes.begin(), seedNodes.end(), node) != seedNodes.end());
}
std::string getNameByIndex(const std::vector<std::pair<std::string,int>>& mapping,
    int index)
{
// Search for the pair whose second == index
for (auto& p : mapping) {
if (p.second == index) {
return p.first; // Return the matching name
}
}
// If not found, return empty or some placeholder
return "";
}
int main()
{
    auto start = std::chrono::high_resolution_clock::now();
    // Input file names
    std::string filenameGraph1 = "rsrc/graph1.el";
    std::string filenameGraph2 = "rsrc/graph2.el";
    std::string filenameSeed   = "rsrc/seed.txt";
    std::string filenameSim    = "rsrc/sim.txt";

    // 1. Build adjacency matrices
    std::vector<std::vector<int>> adjMatrix1;
    std::vector<std::pair<std::string,int>> nodeIndexMapping1 =
        createAdjacencyMatrix(filenameGraph1, adjMatrix1);

    std::vector<std::vector<int>> adjMatrix2;
    std::vector<std::pair<std::string,int>> nodeIndexMapping2 =
        createAdjacencyMatrix(filenameGraph2, adjMatrix2);

    //Display node→index mappings
    //DisplayNodetoIndex(nodeIndexMapping1, filenameGraph1);
    //DisplayNodetoIndex(nodeIndexMapping2, filenameGraph2);

    std::vector<int> SeedNodeGraph1, SeedNodeGraph2;
    // 2. Read seed alignments
    ReadSeed(filenameSeed, nodeIndexMapping1, nodeIndexMapping2, SeedNodeGraph1, SeedNodeGraph2);
    // 3. Read similarity file
    std::vector<std::vector<double>> similarityMatrix = ReadSimFile(nodeIndexMapping1, nodeIndexMapping2, filenameSim);
    
    // 4. Initialize alignment process
    bool alignmentInProgress = true;
    SkipList skiplist(20, 0.5);
    int iterationCount = 0;
    const int maxIterations = 10;
    //calculate candidate:
    std::vector<int> connectedNodes1 = getConnectedNodes(SeedNodeGraph1, adjMatrix1);
    std::vector<int> connectedNodes2 = getConnectedNodes(SeedNodeGraph2, adjMatrix2);
    std::cout << "\nConnected Nodes of Seed Graph 1:\n";
        displayConnectedNodes(connectedNodes1);

    std::cout << "\nConnected Nodes of Seed Graph 2:\n";
        displayConnectedNodes(connectedNodes2);
    //insert candidate to skiplist
    for (int n1 : connectedNodes1) {
        for (int n2 : connectedNodes2) {
            
            if (isNodeAligned(n1, SeedNodeGraph1) || isNodeAligned(n2, SeedNodeGraph2)) {
                continue; // Node is "dead"—already aligned
            }

            double similarity = similarityMatrix[n1][n2];
            
            if (similarity > 0) {
                std::cout << "Inserting: (" << n1 << ", " << n2
                          << ") with similarity " << similarity << "\n";
                skiplist.insertElement(similarity, n1, n2);
            }
        }
    }
    // Main loop: continue until no candidates meet the threshold
    while (alignmentInProgress&& iterationCount < maxIterations) {
        iterationCount++;

        // Pop one candidate from skip list
        auto tup = skiplist.pop(0.1);
        double key;
        int first, second;
        std::tie(key, first, second) = tup;
        // Check if a candidate was found
        if (key != -1.0) {
            std::cout << "Popped node => key: " << key
                      << ", VertexA: " << first
                      << ", VertexB: " << second << "\n";
            // Update seeds for the next iteration
            SeedNodeGraph1.push_back(first);
            SeedNodeGraph2.push_back(second);
            for (size_t i = 0; i < SeedNodeGraph1.size(); ++i) {
                std::cout << "(" << SeedNodeGraph1[i]
                          << ", " << SeedNodeGraph2[i] << ")\n";
            }
                std::vector<int> connectedNodes1 = getConnectedNodes(first, adjMatrix1);
                std::vector<int> connectedNodes2 = getConnectedNodes(second, adjMatrix2);
                std::cout << "\nConnected Nodes of Seed Graph 1:\n";
                    displayConnectedNodes(connectedNodes1);

                std::cout << "\nConnected Nodes of Seed Graph 2:\n";
                    displayConnectedNodes(connectedNodes2);
        
                //insert candidate to skiplist
                for (int n1 : connectedNodes1) {
                    for (int n2 : connectedNodes2) {
                        if (isNodeAligned(n1, SeedNodeGraph1) || isNodeAligned(n2, SeedNodeGraph2)) {
                            continue; // Node is "dead"—already aligned
                        }
                        double similarity = similarityMatrix[n1][n2];
                        if (similarity > 0) {
                         std::cout << "Inserting: (" << n1 << ", " << n2
                          << ") with similarity " << similarity << "\n";
                        skiplist.insertElement(similarity, n1, n2);
                        }
                    }
                }
        }
         else {
            // No more candidates, stop the loop
            std::cout << "[DEBUG] Skiplist is empty, no candidate pairs.\n";
            alignmentInProgress = false;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "\nExecution Time: " << elapsed.count() << " seconds\n";

    // Final output
    std::cout << "\nFinal Alignment (names):\n";

    for (size_t i = 0; i < SeedNodeGraph1.size(); ++i) {
    // Convert each integer index to a name using the helper
    std::string name1 = getNameByIndex(nodeIndexMapping1, SeedNodeGraph1[i]);
    std::string name2 = getNameByIndex(nodeIndexMapping2, SeedNodeGraph2[i]);

    std::cout << "(" << name1 << ", " << name2 << ")\n";
}



    return 0;
}