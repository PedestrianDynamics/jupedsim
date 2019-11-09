Contributing to JuPedSim
========================

This project is mainly developed by a small group of researchers and students from [Jülich Research Center](http://www.fz-juelich.de/en) and [BUW](http://www.uni-wuppertal.de/).
However you are kindly invited not only to use JuPedSim but also contributing to our open-source-project.
It does not matter if you are a researcher, student or just interested in pedestrian dynamics.
There are only a few rules and advices we want to give to you:

- [Advice for new contributors](#advice-for-new-contributors)
    - [First steps](#first-steps)
    - [Workflow](#workflow)
    - [FAQ](#faq)

- [Reporting bugs and requesting features](#reporting-bugs-and-requesting-features)
    - [Using the issue tracker](#using-the-issue-tracker)
    - [Reporting bugs](#reporting-bugs)
    - [Requesting features](#requesting-features)

- [Writing code](#writing-code)
    - [Coding style](#coding-style)
    - [Unit tests](#unit-tests)

- [Writing documentation](#writing-documentation)
    - [Comments](#comments)
    - [Documenting new features](#documenting-new-features)
    - [Sample Templates](#sample-templates)

- [Committing code](#commiting-code)
    - [Committing guidelines](#commiting-guidelines)

## Advice for new contributors

### First steps
* Clone/fork our github/gitlab repository
~~~
git clone https://cst.version.fz-juelich.de/jupedsim/jpscore.git
~~~

* Change to the developement branch and create a branch with your feature.
~~~
git checkout developement 
git checkout -b feature_name
~~~

* Assuming you are in the jpscore folder type 
```
mkdir build && cd build 
cmake .. 
```

* Download all dependencies, check if cmake tells you something is missing
    * Boost >= 1.56
    * Qt
    * Vtk
    
* If everything compiled for the first time, you are free to start

### Workflow
The branches **master** and **develop** are **protected**. You can **only push to your feature-branch.**
Whenever you want to add a feature or fix an issue create a new feature-branch and only work on this branch.
If you change what you wanted to merge the develop-branch into your feature-branch.
Make sure all tests are running after merging and you provided new tests for your feature.
After doing that open a merge/pull request.

If your fix/feature is accepted it will be merged into the develop-branch.

#################
TODO: 
- How to make branches out of issues
- How to work with branches
- Board usage
- Gitlab in general: features
- Wiki vs Web
#######################

### FAQ

## Reporting bugs and requesting features
If you got a question or a problem and need support from our team feel free to contact us.
You can do this via [email](mailto:dev@jupedsim.org).

If you think you found an issue or bug in JuPedSim please use our issue tracker.

### Using the issue tracker

The issue tracker is the preferred channel for bug reports, features requests and submitting pull requests, but please respect the following restrictions:

- Please do not use the issue tracker for personal support requests. [Mail us](mailto:dev@jupedsim.org) if you need personal support.

- Please do not derail or troll issues. Keep the discussion on topic and respect the opinions of others.

If you use the issue tracker we have a list of labels you should use.

### Reporting bugs

If you find a bug in the source code or a mistake in the documentation, you can help us by submitting an issue to our repository.
Even better you can submit a pull or merge request with a fix. Please use the following template and make sure you provide us as much information as possible:

~~~.md
[Short description of problem here]

**Reproduction Steps:**

1. [First Step]
2. [Second Step]
3. [Other Steps...]

**Expected behavior:**

[Describe expected behavior here]

**Observed behavior:**

[Describe observed behavior here]

**Screenshots and GIFs**

![Screenshots and GIFs which follow reproduction steps to demonstrate the problem](url)

**JuPedSim version:** [Enter JuPedSim version here]
**OS and version:** [Enter OS name and version here]
**Compiler and version:** [Enter compiler name and version here]

**Installed Libraries:**
[Enter Boost version here]
[Enter Vtk version here]
[Enter Qt version here]

**Additional information:**

* Problem started happening recently, didn't happen in an older version of JuPedSim: [Yes/No]
* Problem can be reliably reproduced, doesn't happen randomly: [Yes/No]
* Problem happens with all files and projects, not only some files or projects: [Yes/No]
* Problem happens with the attached ini and geometry files: [Yes/No]
~~~

### Requesting features

Enhancement suggestions are tracked as issues. After you've determined which repository your enhancement suggestions is related to, create an issue on that repository and provide the following information:

 * Use a clear and descriptive title for the issue to identify the suggestion.
 * Provide a step-by-step description of the suggested enhancement in as many details as possible.
 * Provide specific examples to demonstrate the steps. Include copy/pasteable snippets which you use in those examples, as Markdown code blocks.
 * Describe the current behavior and explain which behavior you expected to see instead and why.

If you want to support us by writing the enhancement yourself consider what kind of change it is:

- **Major changes** that you wish to contribute to the project should be discussed first on our **dev mailing list** so that we can better coordinate our efforts, prevent duplication of work, and help you to craft the change so that it is successfully accepted into the project.
- **Small changes** can be crafted and submitted to our repository as a **pull or merge request**.

Nevertheless open an issue for documentation purposes with the following template:

~~~.md
[Short description of suggestion]

**Steps which explain the enhancement**

1. [First Step]
2. [Second Step]
3. [Other Steps...]

**Current and suggested behavior**

[Describe current and suggested behavior here]

**Why would the enhancement be useful to most users**

[Explain why the enhancement would be useful to most users]

[List some other text editors or applications where this enhancement exists]

**Screenshots and GIFs**

![Screenshots and GIFs which demonstrate the steps or part of JuPedSim the enhancement suggestion is related to](url)

**JuPedSim Version:** [Enter JuPedSim version here]
**OS and Version:** [Enter OS name and version here]~

~~~

## Writing Code

### Coding style

### Unit Tests

JuPedSim uses [Catch2](https://github.com/catchorg/Catch2) as a unit testing library.
The compilation of unit tests and automatic registration to ctest can be enabled by setting the cmake variable `BUILD_TESTING`:
```sh
cmake -DBUILD_TESTING=ON ..
```

After compilation the unit tests are part of the test suite run by `ctest`
Additionally unit tests can be executed using the `unittests` executable in the `bin` folder.
Use `bin/unittests --help` to see further command line options.

### Writing a unit test

You can use the following as a unit test template:
```c++
#include <catch2/catch.hpp>

TEST_CASE("Module/Classname", "[TestTags]")
{
    SECTION("FunctionName")
    {
    }
}
```
Unit tests are implemented in the folder `test/catch2`.
Please use the tests class name as test name and use section names for the tested use case and function.

### Verification and validation

Besides unit testing, we use in JuPedSim python-driven tests for verification and validation purposes.
Several validation and verification tests for *JuPedSim* (jpscore) are defined in the following section (e.g. Rimea and NIST).
In order to make the nightly builds run automatically, consider the following steps, before adding new tests.
This procedure is also recommended to make simulations with several inifiles e.g. different seeds.

To write additional tests, create a directory under *Utest/*.


- Create in *./jpscore/Utest/* a new direct with a descriptive name. For
   example: */Utest/test\_case1/*
- Put in that directory an ini-file (referred to as "master-inifile")
   and all the relevant files for a simulation, e.g. geometry file, etc. In the master-inifile you can use python syntax
   
   Example:
```xml
   <max_sim_time>[3,4]</max_sim_time> 
   <seed>range(1, 10)</seed>
```
- run the script `makeini.py` with the obligatory option `-f`: Using the
    aforementioned example the call is:
```bash
   python3 makeini.py -f test_case1/inifile.xml
```

The Script is going to create two directories:
- test\_case/trajectories: Here live the simulation trajectories.
- test\_case/inifiles: and here the inifiles, that will be produced  based on the master inifile (in this case test\_case1/inifile.xml).
    Note, that the geometry file and the trajectory files are all relative to the project files in the directory *inifiles*.


- Write a runtest-script. Here you have to define the logic of your test. What should be tested? When does the file succeed or file? etc.

Your script has to start with the following lines:

```python
#!/usr/bin/env python3
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


## Writing Documentation

### Comments
Comments have to be written in **English** everywhere. Please use markdown where applicable.

### Documenting new features
Please update the changelog with every feature/fix you provide so we can keep track of changes for new versions of JuPedSim.

### Sample Templates
```
/*!
 * \file [filename]
 *
 * \author [your name]
 * \date
 * \version     v[x.y]
 * \copyright   <2016-2022> Forschungszentrum Jülich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \section Description
 *
 * [your comment here]
 */
```

```
/*!
 * \class [class name]
 *
 * \brief [brief description]
 *
 * [detailed description]
 *
 * \author [your name]
 * \date
 */
```

```
/*!
 * \brief [brief description]
 *
 * [detailed description]
 *
 * \param[in] [name of input parameter] [its description]
 * \param[out] [name of output parameter] [its description]
 * \return [information about return value]
 * \sa [see also section]
 * \note [any note about the function you might have]
 * \warning [any warning if necessary]
 */
```
