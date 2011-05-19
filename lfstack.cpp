// Test program for Boost proposed Lockfree library: stack container
// Jeff Trull <jetrull@sbcglobal.net> 2011-05-18

#undef NDEBUG

#include <iostream>
#include <stack>

#include <boost/thread.hpp>
#include <boost/lockfree/stack.hpp>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

static const int opcount = 50000000;
static boost::lockfree::stack<int> stack;

static int num_pthreads;
static int num_cthreads;

void producer(int opcount) {
  for (int i = 0; i < opcount; ++i) {
    stack.push(i);
  }
}

void consumer(int opcount) {
  int checksum = 0;    // not really a checksum but will allow us to verify without overflowing
  for (int i = 0; i < opcount; ++i) {
    // spin until stack has contents
    int top;
    while (!stack.pop(&top)) {}
    if (top % 2) {
      checksum += top;
    } else {
      checksum -= top;
    }
  }
  if ((num_pthreads == 1) && (num_cthreads == 1)) {
    // the sum of 0 + 1 - 2 ... + (opcount - 1) assuming opcount is even:
    assert(checksum == (opcount / 2));
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

  // stack should now be empty
  assert(stack.empty());

  return 0;
}
