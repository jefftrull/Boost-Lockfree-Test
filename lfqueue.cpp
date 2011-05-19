// Test program for Boost proposed Lockfree library: fifo queue container
// traditional locking approach (for comparison)
// Jeff Trull <jetrull@sbcglobal.net> 2011-05-18

// we want asserts, always
#undef NDEBUG

#include <iostream>
#include <queue>

#include <boost/thread.hpp>
#include <boost/lockfree/fifo.hpp>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

static const int opcount = 50000000;
static boost::lockfree::fifo<int> queue;

static int num_pthreads;
static int num_cthreads;

void producer(int opcount) {
  for (int i = 0; i < opcount; ++i) {
    queue.enqueue(i);
  }
}

void consumer(int opcount) {
  for (int i = 0; i < opcount; ++i) {
    // spin until queue has contents
    int popval;
    while (!queue.dequeue(&popval)) {}
    if ((num_cthreads == 1) && (num_pthreads == 1)) {
      assert(popval == i);
    }
  }
}

int main(int argc, char **argv) {
  po::options_description optionsdesc("Allowed Options:");
  optionsdesc.add_options()
    ("help", "Describe available options")
    ("pthreads", po::value<int>(&num_pthreads)->default_value(1), "Number of producer threads")
    ("cthreads", po::value<int>(&num_cthreads)->default_value(1), "Number of consumer threads");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, optionsdesc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << optionsdesc << std::endl;
    return 1;
  }

  // launch threads
  std::vector<boost::shared_ptr<boost::thread> > prodthreads;
  for (int i = 0; i < num_pthreads; ++i) {
    prodthreads.push_back(boost::shared_ptr<boost::thread>(new boost::thread(producer, opcount / num_pthreads)));
  }
  std::vector<boost::shared_ptr<boost::thread> > consthreads;
  for (int i = 0; i < num_cthreads; ++i) {
    consthreads.push_back(boost::shared_ptr<boost::thread>(new boost::thread(consumer, opcount / num_cthreads)));
  }

  // wait for all threads to finish
  for (int i = 0; i < num_cthreads; ++i) {
    consthreads[i]->join();
  }
  for (int i = 0; i < num_pthreads; ++i) {
    prodthreads[i]->join();
  }

  // queue should now be empty
  assert(queue.empty());

  return 0;
}
