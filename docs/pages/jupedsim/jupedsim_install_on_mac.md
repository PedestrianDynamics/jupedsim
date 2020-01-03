---
title: Install JuPedSim on Mac
tags: [getting_started, troubleshooting]
keywords: installation
summary: "Installation of JuPeSim on Mac is usually less problematic than on Windows. Easiest way for installation is to use the homebrew"
sidebar: jupedsim_sidebar
permalink: jupedsim_install_on_mac.html
folder: jupedsim
last_updated: Dec 20, 2019
---

{%include tip.html content="to compile from the source code see the instructions for [linux](jupedsim_install_on_linux.html)"%}

## Homebrew installation

Homebrew users can install all packages with the following commands:

### Add tap

First, add jupedsim's tap

```bash
 brew tap JuPedSim/jps
```
{%include note.html content="This step is performed only once."%}


### (optional) Check dependencies

First check the dependencies of the packages you want to install

```bash
 brew info <pkg>
```

with `<pkg>` is one of the available modules:

- `jpseditor`: geometry editor
- `jpscore`: simulation
- `jpsreport`: analysis
- `jpsvis`: visualisation

### Install

then install with

```bash
 brew install --HEAD <pkg>
```

### (optional) Test modules

```bash
 brew test <pkg>
```

### Update module

To update the installed packages use

```bash
 brew upgrade <pkg>
```

or reinstall it with

```bash
 brew reinstall <pkg>
```



{% include links.html %}

