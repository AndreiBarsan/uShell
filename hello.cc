#include <iostream>

// Toy helper program that can be used to test several
// aspects of the shell such as output redirects, correct
// argv passing and return codes.
//
// Prints out the contents of argv and returns the
// integer value of argv[1], if applicable.
int main(int argc, char **argv) {
  using namespace std;
  cout << "Hello, I am the child program!" << endl;
  cout << "argc = " << argc << endl;
  cout << "argv[0] = " << argv[0] << endl;

  int rc = 0;
  if(argc > 1) {
    rc = atoi(argv[1]);
    for(int i = 1; i < argc; ++i) {
      cout << "argv[" << i << "] = " << argv[i] << endl;
    }    
  }

  return rc;
}
