Preconfigured virtual environment
===================================

In order to compile a `JuPedSim` package, one needs to install some required [packages and libraries](requirements), e.g. `boost` or even  a compiler (`g++`) or `cmake`.

Here, `Vagrant` can work some magic and help isolating dependencies and their configuration within a single disposable, consistent environment. 

Therefore, users, whether they are working on Linux, Mac OS X, or Windows, can compile and run code in the same environment, against the same dependencies, all configured the same way. 

To learn more about `Vagrant` this [page](https://docs.vagrantup.com/v2/getting-started/) will get you started.

__`VirtualBox` must be installed on its own prior to using `Vagrant`.__

`VirtualBox` can be installed by [downloading](https://www.virtualbox.org/wiki/Downloads) a package or installer for your operating system and using standard procedures to install that package.

How it works?
============

Basically, we will create, configure and distribute an environment where `JuPedSim` can be
compiled and run without the necessity to pre-install any dependencies.

With `Vagrant` we can create a `Box` that can be used with `VirtualBox` or directly by `Vagrant`. 

Once connected to this _virtual_ machine, the user can compile, run, edit or even git push the code.
 
Download and install Vagrant
============================

Download a binary from the official [webpage](https://www.vagrantup.com/downloads.html).
The installation is straightforward. 


Start out with Vagrant
=====================

In the Terminal 

```
mkdir vagrant_test && cd vagrant_test
```


To start working

```
vagrant init chraibi/jpscore
```

This will download an Ubuntu-image for `Vagrant`, which contains all packages necessary to work with `jpscore`. 

The original image's name is __ubuntu/trusty64__ was modified by installing packages like `g++`, `git`, `cmake` and `boost`.

Start Vagrant
=============

```
vagrant up
```

Connect to the machine 
=====================

```
vagrant ssh
```

This will connect you to your virtual machine. See also the following screenshot


![Screenshot](https://cst.version.fz-juelich.de/jupedsim/jpscore/uploads/df336106129536fbe5257ae1444fcdb0/vagrant.png)


If asked to give a password give `vagrant`.

Testing a box
=============

An alternative use of the _jpscore-vagrant-image_ is to get yourself a "box-file" and open it with `Vagrant` as follows


```
vagrant box add my-box /path/to/the/new.box

vagrant init my-box

vagrant up
```

A box for `jpscore` can be found [here](https://fz-juelich.sciebo.de/index.php/s/5ME7hp7suvQx5JO/download).
