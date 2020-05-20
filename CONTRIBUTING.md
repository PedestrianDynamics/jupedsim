# Contributing to JuPedSim
Thanks for your interest in contributing to JuPedSim.
In this guide we will explain how you can contribute to the development of JuPedSim.

This project is mainly developed by the institute for civil safety and traffic of the [Jülich Research Centre](https://www.fz-juelich.de/ias/ias-7/EN/Home/home_node.html) and the [University of Wuppertal](https://www.asim.uni-wuppertal.de/en.html).
However we very much appreciate contributions from externals to our open-source-project.

## Ways to Contribute
### Bug Reports
If you find a bug in JuPedSim we really want to know about it.
Please try to provide us as much information about the bug as possible and submit a bug report to the github issue tracker using this [link](https://github.com/JuPedSim/jpscore/issues/new?assignees=&labels=Type:%20Bug&template=bug_report.md&title=).
Afterwards we will get back to you as soon as we can.

### Bug Fixes
### Feature Implementation

## How to Submit a Pull Request
The branch **master** is **protected**. You can **only push to your feature-branch.**
Whenever you want to add a feature or fix an issue create a new feature-branch and only work on this branch.
If you change what you wanted to merge the develop-branch into your feature-branch.
Make sure all tests are running after merging and you provided new tests for your feature.
After doing that open a merge/pull request.

If your fix/feature is accepted it will be merged into the develop-branch.

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
