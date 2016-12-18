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



## Install Boost (at least v1.57)

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

or if you are a `brew` user:

```bash
brew install boost
```

Note: Debian's and Ubuntu's install manager offer an old version of Boost, which is not supported by `JuPedSim`.


You can test your Boost installation by using this [minimal example](2016-11-04-boost.html).
