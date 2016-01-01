# Workflow

This is a git-specific workflow that helps you contribute flawlessly you JuPedSim.

- Clone the project (for the first time)

```
    git clone https://cst.version.fz-juelich.de/jupedsim/jpscore.git
```

- Change to the developement branch and create a branch with your feature.

```
    git checkout developement 
    git checkout -b feature_name
```

- Write code and Commit changes

```
    git commit -m "Describe here your changes"
```

- Push your branch to GitLab. But before doing so, [test](pushing) your code.

```
    git push origin feature_name
```
    
- Create a merge request

    a *Master* will review your code & merge it to the developement branch (maybe).

## Tips Working with Git
1. Pull frequently.
2. Commit often, but don’t push every commit.
3. Push when changes are tested.

See also [8 Tips to help you work better with Git](https://about.gitlab.com/2015/02/19/8-tips-to-help-you-work-better-with-git/).
___
**Top Tip**: Your code is maybe fine if you see on the repository's front page this symbol
![symbol](img/success.png) before your commit.

