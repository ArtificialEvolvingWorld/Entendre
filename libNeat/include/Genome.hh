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


#include "cereal/cereal.hpp"
#include "cereal/types/set.hpp"
#include "cereal/types/vector.hpp"
#include "cereal/types/utility.hpp"
#include "cereal/types/unordered_map.hpp"
#include <cereal/types/base_class.hpp>
#include <cereal/access.hpp>


struct NodeGene;
struct ConnectionGene;

class Genome : public uses_random_numbers,
               public requires<Probabilities> {

  friend class cereal::access;

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

  unsigned long LastInnovationNumber() const { return last_innovation; }

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

  /// Generates a random activation function, according the the settings
  /**
     If CPPNs are enabled, uses the relative probabilities of each
     possible node type.  Otherwise, returns a Sigmoid function.
  */
  ActivationFunction GenerateActivationFunction();

  /// Generates a compositional network from a list of Genomes
  /**
     Builds a composite neural network from a list of Genomes. If hetero_inputs is
     true, the resulting networks will have num_inputs x N input nodes. Otherwise,
     each network's inputs will be distributed from a single set of input nodes.
  */
  template<typename NetType>
  friend std::unique_ptr<NeuralNet> BuildCompositeNet(const std::vector<Genome*>& genomes, bool hetero_inputs);

  template<typename Archive>
  void serialize(Archive& ar) {
    ar( num_inputs,
	num_outputs,
	node_genes,
	node_lookup,
	connection_genes,
	connection_lookup,
	connections_existing,
	last_innovation,
	cereal::base_class< requires<Probabilities> >( this )
     );
  }

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

  
  template <class Archive>
  void serialize( Archive & ar ) {
    ar( innovation, origin, dest, weight, enabled );
  }
  
};

struct NodeGene {

  friend cereal::access;
  
  NodeGene() {;}
  //NodeGene() = delete;
  NodeGene(NodeType type,
           ActivationFunction func = ActivationFunction::Sigmoid,
           unsigned long innovation = 0)
    : type(type), func(func), innovation(innovation) { }

  NodeType type;
  ActivationFunction func;
  unsigned long innovation;

private:
  
  template <class Archive>
  void serialize( Archive & ar ) {
    ar( type, func, innovation );
  }

};

CEREAL_CLASS_VERSION(Genome, 1)

