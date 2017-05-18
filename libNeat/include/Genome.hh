#pragma once

#include <ostream>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <vector>
#include <memory>

#include "Random.hh"
#include "Requirements.hh"
#include "ReachabilityChecker.hh"
#include "ConsecutiveNeuralNet.hh"
#include "ConcurrentNeuralNet.hh"
#include "ConcurrentGPUNeuralNet.hh"


struct NodeGene;
struct ConnectionGene;

class Genome : public uses_random_numbers,
               public requires<Probabilities> {

  template<typename NetType>
  friend std::unique_ptr<NeuralNet> BuildCompositeNet(const std::vector<Genome*>& genomes, bool hetero_inputs);


public:

  Genome();
  static Genome ConnectedSeed(int num_inputs, int num_outputs,
                              ActivationFunction func = ActivationFunction::Sigmoid);

  template<typename NetType>
  std::unique_ptr<NeuralNet> MakeNet() const {
    std::unique_ptr<NeuralNet> output = std::make_unique<NetType>();
    MakeNet(*output);
    return output;
  }

  Genome& operator=(const Genome&);
  Genome& AddNode(NodeType type, ActivationFunction func=ActivationFunction::Sigmoid);
  Genome& AddConnection(unsigned long origin, unsigned long dest,
                        bool status, double weight);
  Genome& RandomizeWeights();
  Genome  MateWith(const Genome& father);
  Genome  MateWith(Genome* father);
  void    Mutate();
  void    MutateConnection();
  void    MutateNode();
  void    MutateWeights();
  void    MutateReEnableGene();
  void    MutateToggleGeneStatus();
  bool    ConnectivityCheck(unsigned int node_index, const ReachabilityChecker& checker) const;
  float   GeneticDistance(const Genome&) const;
  Genome  GeneticAncestry() const;
  void    PrintInnovations() const;
  size_t  Size() const { return connection_genes.size(); }
  size_t  NumInputs () const { return num_inputs;  }
  size_t  NumOutputs() const { return num_outputs; }

  bool IsStructurallyEqual(const Genome& other) const;

  friend std::ostream& operator<<(std::ostream&, const Genome& genome);

  void AssertNoDuplicateConnections() const;
  void AssertInputNodesFirst() const;
  void AssertNoConnectionsToInput() const;

private:
  void MakeNet(NeuralNet& net) const;

  const NodeGene* GetNodeByN(unsigned int i) const;
  const NodeGene* GetNodeByInnovation(unsigned long innovation) const;
  const ConnectionGene* GetConnByN(unsigned int i) const;
  const ConnectionGene* GetConnByInnovation(unsigned long innovation) const;

  NodeGene* GetNodeByN(unsigned int i);
  NodeGene* GetNodeByInnovation(unsigned long innovation);
  ConnectionGene* GetConnByN(unsigned int i);
  ConnectionGene* GetConnByInnovation(unsigned long innovation);


  /// Adds the node gene given
  /**
     If a node already exists with that innovation number,
       this function has no effect.
   */
  void AddNodeGene(NodeGene gene);

  /// Adds the connection gene given
  /**
     If the connection is invalid, this function has no effect.
     In order to be valid, the following conditions must be met.
     - The origin and destinations nodes must exist.
     - No connection exists with the same origin and destination.
     - No connection exists with identical innovation number.
     - The destination node is not an input node.
   */
  void AddConnectionGene(ConnectionGene gene);

  Genome& AddNodeByInnovation(NodeType type, ActivationFunction func,
                              unsigned long innovation);
  void AddConnectionByInnovation(unsigned long origin, unsigned long dest,
                                 bool status, double weight);

private:
  size_t num_inputs;
  size_t num_outputs;

  std::vector<NodeGene> node_genes;
  std::unordered_map<unsigned long,unsigned int> node_lookup;

  std::vector<ConnectionGene> connection_genes;
  std::unordered_map<unsigned long,unsigned int> connection_lookup;
  std::set<std::pair<unsigned long, unsigned long> > connections_existing;

  // innovation record keeping
  unsigned long last_innovation;
};


struct ConnectionGene {
  unsigned long innovation;
  unsigned long origin;
  unsigned long dest;
  double weight;
  bool enabled;
  bool operator==(const ConnectionGene& other) {
    return (origin == other.origin) && (dest == other.dest);
  }
};

struct NodeGene {
  NodeGene() = delete;
  NodeGene(NodeType type,
           ActivationFunction func = ActivationFunction::Sigmoid,
           unsigned long innovation = 0)
    : type(type), func(func), innovation(innovation) { }

  NodeType type;
  ActivationFunction func;
  unsigned long innovation;
};

