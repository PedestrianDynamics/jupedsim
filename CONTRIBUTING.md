# Contributing to JuPedSim
Thanks for your interest in contributing to JuPedSim.
In this guide we will explain how you can contribute to the development of JuPedSim.

This project is mainly developed by the institute for civil safety and traffic of the [Jülich Research Centre](https://www.fz-juelich.de/ias/ias-7/EN/Home/home_node.html) and the [University of Wuppertal](https://www.asim.uni-wuppertal.de/en.html).
However we very much appreciate contributions from externals to this open-source-project.

## Ways to Contribute
### Bug Reports
If you find a bug in JuPedSim we really want to know about it.
Please try to provide us as much information about the bug as possible and submit a bug report to the github issue tracker using this [link](https://github.com/JuPedSim/jpscore/issues/new?assignees=&labels=Type:%20Bug&template=bug_report.md&title=).
Afterwards we will get back to you as soon as we can.

### Bug Fixes & Feature Implementation
If you want to fix a bug or extend JuPedSim with a certain feature feel free to contact us via the issue tracker.
We can discuss the desired implementation of the bugfix or feature upfront if you wish.
If you want to get early feedback you can create a draft pull request.
During the review process we will help you fulfill the coding standards and guidelines.

## How to Submit a Pull Request
To contribute any code to JuPedSim you have to create a Pull Request.
For this purpose please fork our repository and start developing in your personal fork.
It would be best practice to create a feature branch in your own fork, but working on master also works.
After you pushed the first changes you can already create a draft pull request.

After a pull request is opened we will have a look on the change-set and start the continuous integration suite.
To merge the pull request the following conditions need to be fulfilled:
- All tests in the CI must be successful and
- The PR needs to be accepted by at least one reviewer from the main developers of JuPedSim.

The CI contains the compilation of JuPedSim on the following platforms and compilers:
- Windows (MSVC-19)
- Linux (GCC-8 and Clang-8)
- MacOS (AppleClang-11)

Additionally, all unittest and a selection of integration tests are run on the mentioned platforms.

## Coding Standards

### Format

### Coding Guidelines

## Testing

### Unit Tests
### Integration Tests

Testing should be enabled by cmake as follows:
```javascript
cmake -DBUILD_CPPUNIT_TEST=ON ..
```

After compiling (`make`) the tests can be called as follows:

```javascript
ctest
```
## Writing Documentation

### Comments
Comments have to be written in **English** everywhere. Please use markdown where applicable.

### Documenting new features
Please update the changelog with every feature/fix you provide so we can keep track of changes for new versions of JuPedSim.
