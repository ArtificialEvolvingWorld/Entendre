#include <cereal/types/unordered_map.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/archives/binary.hpp>
#include "Genome.hh"

#include "mpi.h"



#include <istream>
#include <chrono>
#include <thread>

#include <sys/types.h>
#include <unistd.h>



struct membuf: std::streambuf {
  membuf(char const* base, size_t size) {
    char* p(const_cast<char*>(base));
    this->setg(p, p, p + size);
  }
};

struct imemstream: virtual membuf, std::istream {
  imemstream(char const* base, size_t size)
    : membuf(base, size)
    , std::istream(static_cast<std::streambuf*>(this)) {
  }
};

struct my_stringbuf : std::stringbuf {
  const char* my_str() const { return pbase(); } // pptr might be useful too
};



int main(int argc, char** argv) {
  int mpi_thread_support[1];
  MPI_Init_thread(&argc,&argv,MPI_THREAD_SINGLE,mpi_thread_support);
  int size; MPI_Comm_size(MPI_COMM_WORLD, &size);
  int rank; MPI_Comm_rank(MPI_COMM_WORLD, &rank);




  if (rank==0) {

    std::ostringstream os(std::ios::binary);
    {
      auto mother = Genome()
        .AddNode(NodeType::Bias)
        .AddNode(NodeType::Input)
        .AddNode(NodeType::Hidden)
        .AddNode(NodeType::Output)
        .AddConnection(0,3,true,1.)
        .AddConnection(1,3,true,1.)
        .AddConnection(1,2,true,1.)
        .AddConnection(2,3,true,1.);
      mother.set_generator(std::make_shared<RNG_MersenneTwister>());
      mother.required(std::make_shared<Probabilities>());
      auto father = mother;
      mother.AddConnection(3,2,true,1.);
      father.AddConnection(0,2,true,1.);
      auto child = mother.MateWith(father);
      cereal::BinaryOutputArchive ar( os );
      std::vector<Genome> pop;
      for (int i = 0; i<100; i++) {
        pop.push_back(child.RandomizeWeights());
      }
      ar( pop );
    }
    // TODO: copy should be optimized to a buffer move
    auto data_copy = os.str();
    const char* data = data_copy.c_str();


      MPI_Request req;
      int packet_size = data_copy.size();
      MPI_Isend((const void*)&packet_size,1,MPI_INT,1,0,MPI_COMM_WORLD,&req);
      MPI_Isend((const void*)data,packet_size,MPI_CHAR,1,0,MPI_COMM_WORLD,&req);
  } else if (rank==1) {
    int packet_size = 0;
    MPI_Request req;
    MPI_Irecv((void*)&packet_size,1,MPI_INT,0,0,MPI_COMM_WORLD,&req);
    int completed = 0;

    while(!completed) {
      MPI_Test(&req,&completed,MPI_STATUS_IGNORE);
      /* continue to do calculations until completed */
    }
    char* data = new char[packet_size];
    MPI_Recv((void*)data,packet_size,MPI_BYTE,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);


    // copy buffer into a string stream (TODO: need a stream that avoids the copy)
    std::stringstream is;
    is.write(data,packet_size*sizeof(char));

    std::vector<Genome> pop2;
    //std::cout << packet_size*sizeof(char) << std::endl;
    {
      int test = 1;
      cereal::BinaryInputArchive ar( is );
      ar( pop2 );
    }
    std::cout << "Deserialized pop size: " << pop2.size() << std::endl;
    std::cout << "Bytes sent: " << packet_size << std::endl;
    delete data;
  }














  MPI_Finalize();

  return 0;
}
