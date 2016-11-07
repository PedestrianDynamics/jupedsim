---
layout: post
title: Quick start
subtitle: Compiling the code
permalink: 2016-11-02-quickstart.html
---

To install `JuPedSim` there are three steps to follow: 

# 1. Get the code

```bash
 git clone https://github.com/JuPedSim/JuPedSim.git
 cd JuPedSim
 git submodule update --init --recursive
```

# 2. Check the requirements 
Some packages and libraries are required to get `JuPedSim` compiled. See also [requirements](2016-11-03-requirements.html).

You can check your system with 

```bash 
 make -f Makefile.cmake check
```

# 3. Compile the code

If the two first steps succeed then you can proceed with the compilation 

```bash 
make -f Makefile.cmake
```

In case of successful compilation,  the executables will be located in the directory `bin/`.
