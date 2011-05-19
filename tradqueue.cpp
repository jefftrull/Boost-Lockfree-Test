// Test program for Boost proposed Lockfree library: fifo queue container
// traditional locking approach (for comparison)
// Jeff Trull <jetrull@sbcglobal.net> 2011-05-18

#undef NDEBUG

#include <iostream>
#include <queue>

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

static const int opcount = 50000000;
static std::queue<int> queue;
static boost::mutex shared_lock;

static int num_pthreads;
static int num_cthreads;

void producer(int opcount) {
  int produced_count = 0;
  while (produced_count < opcount) {
    // lock
    boost::mutex::scoped_lock sl(shared_lock);
    queue.push(produced_count++);
    // unlock
  }
}

void consumer(int opcount) {
  int consumed_count = 0;
  while (consumed_count < opcount) {
    // lock
    boost::mutex::scoped_lock sl(shared_lock);
    if (!queue.empty()) {
      int front = queue.front();
      if ((num_pthreads == 1) && (num_cthreads == 1)) {
	assert(front == consumed_count);
      }
      queue.pop();
      consumed_count++;
    }
    // unlock
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
