#include <algorithm>
#include <random>
#include <chrono>
#include <cmath>
#include <iostream>

#include "make_unique.hh"
#include "Population.hh"
#include "XorFitnessEvaluator.hh"
#include "Timer.hh"
#include "dummy.hh"
#include "ArgParser.hh"




auto population_from_xor_experiment(unsigned int pop_size = 2000, unsigned int num_generations=10) {
  auto seed = Genome::ConnectedSeed(2,1);

  auto prob = std::make_shared<Probabilities>();
  prob->new_connection_is_recurrent = 0;
  prob->keep_non_matching_father_gene = 0;
  prob->population_size = pop_size/2;
  prob->number_of_children_given_in_nursery = pop_size/2;

  Population pop(seed,
                 std::make_shared<RNG_MersenneTwister>(),
                 prob);

  //pop.SetNetType<ConsecutiveNeuralNet>();
  pop.SetNetType<ConcurrentNeuralNet>();
  //pop.EnableCompositeNet(/*hetero_inputs = */true);


  for (unsigned int generation = 0u; generation < num_generations; generation++) {
    pop = std::move(pop.Reproduce([](){return make_unique<XorFitness>();}));

  }
  return pop;
}


int main(int argc, char** argv) {

  bool help = false;
  unsigned int num_trials;
  unsigned int pop_size;
  unsigned int num_networks;
  unsigned int num_gen;

  /////////////////////////////////////////////////////////////////////////////////////
  ArgParser parser;
  parser.option("p population_size", &pop_size)
    .description("Number of networks in population.")
    .default_value(1024);
  parser.option("n num_networks", &num_networks)
    .description("Number of networks to evaluate.")
    .default_value(1024);
  parser.option("N num_trials", &num_trials)
    .description("Number of trials for performance evaluation.")
    .default_value(100);
  parser.option("g num_generations", &num_gen)
    .description("Number of generations to evolve networks over")
    .default_value(20);
  parser.option("h help ?", &help)
    .description("Show the program options help menu.");
  parser.parse(argc,argv);
  if (help) { std::cout << parser << std::endl; return 0;}
  /////////////////////////////////////////////////////////////////////////////////////


  auto xor_pop = std::move(population_from_xor_experiment(pop_size,num_gen));
  auto xor_genomes = xor_pop.GetPopulation();
  std::cout << "/* Seed population built (size: " << xor_genomes.size() << ") */" << std::endl;


  std::vector<Genome> xor_genomes_expanded;
  xor_genomes_expanded.reserve(num_networks);

  for (auto i=0u; i<num_networks; i++) {
    xor_genomes_expanded.push_back(*xor_genomes.at(i%xor_genomes.size()));
  }
  xor_genomes.clear();
  xor_genomes.reserve(xor_genomes_expanded.size());
  for (auto& genome : xor_genomes_expanded) {
    genome.RandomizeWeights();
    xor_genomes.push_back(&genome);
  }
  std::cout << "/* Expanded genome set built (size: " << xor_genomes.size() << ") */" << std::endl;


  std::vector<_float_> ompoutputs;
  float tomp = 0.0;
  {
    double tperformance = 0.0;
    //----------------------------------------------------------------------------------
    std::unique_ptr<NeuralNet> networks = nullptr;
    {
      Timer teval([&tperformance](auto elapsed) { tperformance+=elapsed; });
      networks = BuildCompositeNet<ConcurrentKNLNeuralNet>(xor_genomes,false);
    } std:: cout << tperformance/1.0e6 << " ms" << " for construction of composite network. " << std::endl;



    std::vector<_float_> inputs = {1.,1.};
    std::vector<_float_> outputs;
    outputs = networks->evaluate(inputs);
    dummy(outputs);

    tperformance = 0.0;
    for (auto i=0u; i<num_trials; i++ ){
      Timer teval([&tperformance](auto elapsed) { tperformance+=elapsed; });
      outputs = networks->evaluate(inputs);
      dummy(outputs);
    } std:: cout << tperformance/num_trials/1.0e6 << " ms" << " for omp net evaluation. " << std::endl;
    ompoutputs = std::move(outputs);
    tomp = tperformance/num_trials/1.0e6;
  }

  std::vector<_float_> serialoutputs;
  float tserial = 0.0;
  {
    double tperformance = 0.0;
    //----------------------------------------------------------------------------------
    std::unique_ptr<NeuralNet> networks = nullptr;
    {
      Timer teval([&tperformance](auto elapsed) { tperformance+=elapsed; });
      networks = BuildCompositeNet<ConcurrentNeuralNet>(xor_genomes,false);
    } std:: cout << tperformance/1.0e6 << " ms" << " for construction of composite network. " << std::endl;



    std::vector<_float_> inputs = {1.,1.};
    std::vector<_float_> outputs;
    outputs = networks->evaluate(inputs);
    dummy(outputs);

    tperformance = 0.0;
    for (auto i=0u; i<num_trials; i++ ){
      Timer teval([&tperformance](auto elapsed) { tperformance+=elapsed; });
      outputs = networks->evaluate(inputs);
      dummy(outputs);
    } std:: cout << tperformance/num_trials/1.0e6 << " ms" << " for serial net evaluation. " << std::endl;
    serialoutputs = std::move(outputs);
    tserial = tperformance/num_trials/1.0e6;
  }

  //----------------------------------------------------------------------------------



  for (auto i=0u; i<ompoutputs.size(); i++) {
    assert(std::abs(ompoutputs[i]-serialoutputs[i]) < 1e-4);
  }
  std::cout << "/* PASS: outputs from KNL and CPU implementations are identical. */" << std::endl;
  std::cout << "~*~*~*~* KNL speed up: " << tserial/tomp << " *~*~*~*~" <<std::endl << std::endl;
  //std::cout << *serial_networks << std::endl;





  return 0;
}
