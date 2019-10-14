# Contributing

This page provides a starting point for getting involved with the JuPedSim/jpsreport project.

There are several ways to contribute to a project with Git. These are guidelines, not laws. But following this workflow adds a level of quality assurance to the project. Notes describing how to perform specific tasks using Git are given here.

Please open a new [issue](https://github.com/JuPedSim/jpsreport/issues) if you encouter problems or have found a bug in the code.

## Getting started

### Install Git on your local machine

Mac and Linux users will likely already have Git installed. To confirm this, open a terminal and type

```
$git --version
```

To install Git for windows go [here](https://git-scm.com/download/win).

### Create a GitHub account

If you do not already have one, create a [GitHub account](https://github.com/).

### Do some local configuration

You will need to set your [Git configuration](http://www.git-scm.com/docs/git-config) so that the remote server knows who you are when you are pushing.

**Your username** (need not be your GitHub username):

```
$ git config --global user.name "Paul Mustermann"
```

**Your e-mail adress** (same as your GitHub account):

```
$ git config --global user.email "paul@mustermann.com"
```

## Fork repos from JuPedSim

Go to the [JuPedSim](https://github.com/JuPedSim) organization GitHub page. For each project you want to work with, fork the corresponding repo(s) over to your GitHub account by clicking the fork icon on the upper right. [Here](https://help.github.com/en), you can find more information about [forking](https://help.github.com/en/articles/fork-a-repo).


## Clone the forked repos

The repo you will work from is now in your space on GitHub. To get this repo to your computer you need to clone it. There are a few ways to do this. For example, see the "Clone to Desktop" and "Download Zip" buttons to the right side of your project page. Here I will use the command line SSH clone. Note that you need to have an SSH key enabled for your computer. [Here](https://help.github.com/en), you can find more information about [cloning](https://help.github.com/en/articles/cloning-a-repository-from-github).


## Staying up-to-date with the central repo

First, check your remote state. You should see no "upstream" repositories. The only remote repo you should see is origin.

```
$ git remote -v
origin git@github.com:username/jpsreport.git (fetch)
origin git@github.com:username/jpsreport.git (push)
```

If you see "upstream" repos in addition to origin, then you need to remove them.

```
$ git remote rm <repo-name>
```

Add tracking for JuPedSim/jpsreport. Here we will go ahead and name our "upstream" repo "JuPedSim".

```
$ git remote add JuPedSim git://github.com/JuPedSim/jpsreport.git
```

## Updating your repository

To get all the changes committed by other project members to the central repo, do the following:

```
$ git remote update
Fetching origin
Fetching JuPedSim
```

Make sure, you are in your develop branch
```
$ git checkout develop
```

See what different in your development branch
```
$ git diff JuPedSim/develop
```

Merge any changes from the central repo
```
$ git merge JuPedSim/develop
```

Your LOCAL repo will now have changes made to JuPedSim/jpsreport. However, your GitHub repo will still be behind. You need to push the changes up to GitHub.
```
$ git push origin develop
```

## Making changes

When you're working on the JuPedSim/jpsreport project, you're going to have a bunch of different features or ideas in progress at any given time – some of which are ready to go, and others which are not. Branching exists to help you manage this workflow. When you create a branch in your project, you're creating an environment where you can try out new ideas. Changes you make on a branch don't affect the main develop branch, so you're free to experiment and commit changes, safe in the knowledge that your branch won't be merged until it's ready to be reviewed by someone you're collaborating with.

The next step is to "stage" your changes for commit and then to "commit" the changes to your local repo.

**But wait! First, make sure your working branch is up-to-date by fetching and merging (pulling) any last-minute changes from the central repo.**

```
$git fetch JuPedSim
```

```
$ git diff JuPedSim/develop
```

Are these the changes you except to see? **If not, don't merge.**

```
$ git merge JuPedSim/develop
```

Now do your [add](https://www.git-scm.com/docs/git-add) and [commit](https://www.git-scm.com/docs/git-commit).

You are now going to push your commit up to your GitHub repo. It is good to explicitly say which repo and which branch you are pushing to, else you can forget where you are (if you are, say, a topic branch instead) and get your repo in a state that you have to untangle.

```
git push origin develop
```

Now your changes are in your repo on GitHub. It's time to submit a pull request.

## Submit a pull request

Go to your account on GitHub. Make sure you are on your develop branch. You should see a note saying something like "This branch is 1 commit ahead of JuPedSim:develop" (of course, this will depend on the current state of each repo). Next, click the "Compare & pull request" button. Notice you are now on the JuPedSim/jpsreport account page. Use the compare tool to double check the changes you are submitting are the ones you intend to submit. It is also possible to just see a list of the files you are touching.

## Accept or reject the request

Pull requests are accepted or rejected by project members. You can accept your own pull request if you are a member of the JuPedSim organization. The four-eye principle is desirable.

### Checking Out GitHub Pull Requests Locally

To check out GitHub pull requests locally, use this command:

```
git fetch JuPedSim pull/$1/head:pr$1
git checkout pr$1
```

The variable `$1` stands for the number of the pull request.
