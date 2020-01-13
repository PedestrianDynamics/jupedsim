# Juelich Tests
Note that a detailed description of the test cases can be found at
<http://www.jupedsim.org/jpscore/2016-11-02-juelich>

While most full system tests in the jpscore test suite contain geometry / ini
files to run `jpscore` several test cases in this part of the test suite
generate ini files based on a template. The template is usually called
`master_ini.xml` and contains hints how to generate permutations of test input.

`master_ini.xml` files are processed by `Utest/makeini.py`, see this file for
details.
