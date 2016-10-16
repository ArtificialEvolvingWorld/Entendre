#pragma once

#include <vector>
#include <ostream>
#include <unordered_map>

#include "NeuralNet.hh"
#include "Random.hh"
#include "Requirements.hh"
#include "insertion_preserving_unordered.hh"

struct ConnectionGene {
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
  NodeGene(const NodeType& type_) : type(type_), innovation(0) {;}
  NodeGene(const NodeType& type_, unsigned long innov) : type(type_), innovation(innov) {;}
  NodeType type;
  unsigned long innovation;
};

class ReachabilityChecker;

class Genome : public uses_random_numbers,
               public requires<Probabilities> {
public:
  Genome();
  operator NeuralNet() const;
  Genome& operator=(const Genome&);
  Genome& AddNode(NodeType type);
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
  size_t  Size() { return connection_genes.size(); }

  bool IsStructurallyEqual(const Genome& other) const;

  friend std::ostream& operator<<(std::ostream&, const Genome& genome);

private:
  Genome& AddNode(NodeType type, unsigned long innovation);
  void AddConnectionByInnovation(unsigned long origin, unsigned long dest,
                                 bool status, double weight);

  static unsigned long Hash(unsigned long origin,unsigned long dest,unsigned long previous_hash) {
    return ((origin*746151647UL) xor (dest*15141163UL) xor (previous_hash*94008721UL) xor (5452515049UL));
  }
  static unsigned long Hash(unsigned long id,unsigned long previous_hash) {
    return ((id*10000169UL) xor (previous_hash*44721359UL) xor (111181111UL));
  }

  void AssertNoDuplicateConnections() const;

private:
  size_t num_inputs;
  std::vector<NodeGene> node_genes;
  std::unordered_map<unsigned long,unsigned int> node_lookup;
  insertion_ordered_map<unsigned long, ConnectionGene> connection_genes;


  // innovation record keeping
  unsigned long last_conn_innov;
  unsigned long last_node_innov;
};
