# Unit testing, verification and validation tests

Testing should be enabled by cmake as follows:
```
cmake -DBUILD_TESTING=ON ..
```

After compiling (`make`) the tests can be called as follows:

```
ctest
```

This will take some hours time (depending on your machine). Threfore, a quick testing could be used:
```
ctest -R Boost
```

which run a limited set of tests on the code.

## Writing a unit test
If you write a unit test the first lines in your file should be

```c++
#define BOOST_TEST_MODULE MyTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include classtotest.h

BOOST_AUTO_TEST_SUITE(MyTestClass)
```

Then you can start implementing your test cases by using
```c++
BOOST_AUTO_TEST_CASE(MyTestCase) 
{
  ...
  }
```

For each method or function you want to test you should write a new test case and give it a speaking name.

## Testing the functionality of a model: writing a test with python
Besides unit testing, we use in JuPedSim python-driven tests for verification and validation purposes.
Several benchmarks are implemented e.g. Rimea and NIST.

To write additional tests, create a directory under *Utest/*.
Files your test directory should contain are:

1. geometry.xml
2. init_test.xml
3. master_ini.xml
4. your_runtest.py

Your script has to start with the following lines:

```python
#!/usr/bin/env python
import os
import sys
from sys import *
sys.path.append(os.path.abspath(os.path.dirname(sys.path[0])))
from JPSRunTest import JPSRunTestDriver
from utils import *
```

After including these lines you can write the test you want to perform for every ini-file generated from the makeini.py script.
The method signature must contain at least two arguments to receive the inifile and the trajectory file from the simulation.

```python
def runtest(inifile, trajfile):
```

You can also use more than these arguments like this:

```python
def runtest(inifile, trajfile, x, y, z):
```

If you need a more complex example of how to use more arguments for further calculations please see runtest_14.py.
If a test has to fail because an error occurs or a necessary condition is not fulfilled  you can simply exit the script by using something like

```python
if condition_fails: 
  exit(FAILURE)
```
Once you have written your test you have to make your script executable, so it has to contain a main function which calls the test:

```python
if __name__ == "__main__":
  test = JPSRunTestDriver(1, argv0=argv[0], testdir=sys.path[0])
  test.run_test(testfunction=runtest)
  logging.info("%s exits with SUCCESS" % (argv[0]))
  exit(SUCCESS)
```


