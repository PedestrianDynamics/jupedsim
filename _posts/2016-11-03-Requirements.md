---
layout: post
title: Requirements
subtitle: Dependencies of JPScore
#bigimg: /img/jupedsim_small.png
permalink: 2016-11-03-requirements.html
---

In order to compile `JuPedSim` it is necessary to first install the required libraries. 

## Supported compilers

Any compiler with support for C++11. 

Tested with   

- `g++` (linux/osx)
- `clang` (osx)
- Visual Studio 2013 (Windows)

## Required tools 

- cmake: see this [tutorial](https://www.johnlamp.net/cmake-tutorial-1-getting-started.html) for a brief overview.  
- Python (highly recommended): needed to plot results of analysis. 

## Required libraries 

- [Boost library](http://www.boost.org/): necessary for `jpscore` and `jpsreport`



## Install Boost (at least v1.59)

### Mac

For `brew` users:

```bash
brew install boost
```

And for `port` users

```bash
sudo port install boost 
```

### Linux 

You can compile boost using the following snippet:

```bash
boost_version=1.61.0
boost_dir=boost_1_61_0

wget http://downloads.sourceforge.net/project/boost/boost/${boost_version}/${boost_dir}.tar.gz
tar xfz ${boost_dir}.tar.gz
rm ${boost_dir}.tar.gz
cd ${boost_dir}
./bootstrap.sh --with-libraries=filesystem,test,system
sudo ./b2 --without-python --prefix=/usr -j 4 link=shared runtime-link=shared install
cd ..
rm -rf ${boost_dir}
sudo ldconfig
```

(download this snippet as a [script](https://gitlab.version.fz-juelich.de/jupedsim/jpscore/snippets/7)).


Note: Debian's and Ubuntu's install manager offer an old version of Boost, which is not supported by `JuPedSim`.



### Windows 

```
bootstrap
b2  variant=release --build-type=complete 
```

See also [Getting started on Windows](http://www.boost.org/doc/libs/1_65_1/more/getting_started/windows.html).

This [script](https://gitlab.version.fz-juelich.de/jupedsim/jpscore/snippets/18) can be useful, in case you are using Visual Studio.

Download it and put it in the same directory as Boost. Depending on your Boost version and VS, 
you may want to adapt in the script the variables `boost_dir` and `msvcver`.

## Test Boost installation 

You can test your Boost installation by using this [minimal example](http://www.jupedsim.org/jpscore/2016-11-04-boost.html).
