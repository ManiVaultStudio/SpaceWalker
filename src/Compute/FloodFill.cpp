#include "FloodFill.h"

FloodFill::FloodFill(int numWaves) :
    _numWaves(numWaves),
    _lastKnnGraph(nullptr),
    _lastSelectedPoint(0),
    _lastNumWaves(numWaves)
{

}

void FloodFill::setNumWaves(int numWaves)
{
    _numWaves = numWaves;
    recompute();
}

void FloodFill::compute(const KnnGraph& knnGraph, nint selectedPoint)
{
    int numNeighbours = knnGraph.getNumNeighbours();
    auto& neighbours = knnGraph.getNeighbours();
    nint numPoints = neighbours.size();

    _waves.clear();
    _waves.resize(_numWaves);

    // Set list of nodes to process next, skipping iteration 0 where we just process the seed node
    std::vector<nint> currentNodes = neighbours[selectedPoint];

    // List of nodes that become the new current nodes
    std::vector<nint> newNodes;

    // List of nodes that have been visited during the process
    std::vector<bool> visitedNodes(numPoints, false);

    // Set the seed node as having been visited as we skip iteration 0
    visitedNodes[selectedPoint] = true;
    _waves[0].push_back(selectedPoint);

    // Start flooding in N waves, skipping iteration 0 where we would just process the seed node
    for (int w = 1; w < _numWaves; w++)
    {
        // Reserve the maximum possible nodes that can be added in advance to save on resizing costs
        _waves[w].reserve(currentNodes.size());

        // Process list of current nodes, if they hadn't been visited yet, add them to the flood fill,
        // otherwise skip further processing.
        for (nint& currentNode : currentNodes)
        {
            if (!visitedNodes[currentNode])
            {
                visitedNodes[currentNode] = true;
                _waves[w].push_back(currentNode);
            }
            // Node was visited before, stop further processing
            else
                currentNode = -1;
        }
        
        if (w == _numWaves - 1)
            continue;

        // Add neighbours of just visited current nodes to new nodes
        newNodes.clear();
        newNodes.reserve(currentNodes.size() * numNeighbours);
        for (const nint& node : currentNodes)
        {
            // Node was visited before, skip adding neighbours
            if (node == -1) continue;

            newNodes.insert(newNodes.end(), neighbours[node].begin(), neighbours[node].end());
        }

        // New nodes become the current indices
        currentNodes = newNodes;
    }

    // Compute flat vector of all nodes
    _allNodes.clear();
    for (int w = 0; w < getNumWaves(); w++)
        _allNodes.insert(_allNodes.end(), _waves[w].begin(), _waves[w].end());

    // Store input parameters for potential recomputation
    _lastKnnGraph = &knnGraph;
    _lastSelectedPoint = selectedPoint;
    _lastNumWaves = _numWaves;
}

void FloodFill::recompute()
{
    if (_numWaves == _lastNumWaves) return;

    if (_numWaves < _lastNumWaves)
    {
        _waves.resize(_numWaves);
        return;
    }

    if (_lastKnnGraph == nullptr) return;

    compute(*_lastKnnGraph, _lastSelectedPoint);
}
