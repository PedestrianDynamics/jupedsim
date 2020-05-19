Contributing to JuPedSim
========================

This project is mainly developed by a small group of researchers and students from [Jülich Research Center](https://www.fz-juelich.de/ias/ias-7/EN/Home/home_node.html) and [BUW](https://www.asim.uni-wuppertal.de/en.html).
However you are kindly invited not only to use JuPedSim but also contributing to our open-source-project.
It does not matter if you are a researcher, student or just interested in pedestrian dynamics.

## Workflow
The branch **master** is **protected**. You can **only push to your feature-branch.**
Whenever you want to add a feature or fix an issue create a new feature-branch and only work on this branch.
If you change what you wanted to merge the develop-branch into your feature-branch.
Make sure all tests are running after merging and you provided new tests for your feature.
After doing that open a merge/pull request.

If your fix/feature is accepted it will be merged into the develop-branch.

## Reporting bugs and requesting features
If you got a question or a problem and need support from our team feel free to contact via the issue tracker.

### Reporting bugs

If you find a bug in the source code or a mistake in the documentation, you can help us by submitting an issue to our repository.
Even better you can submit a pull or merge request with a fix. Please use the following template and make sure you provide us as much information as possible:

~~~.md
[Short description of problem here]

**Reproduction Steps:**

1. [First Step]
2. [Second Step]
3. [Other Steps...]

**Expected behavior:**

[Describe expected behavior here]

**Observed behavior:**

[Describe observed behavior here]

**Screenshots and GIFs**

![Screenshots and GIFs which follow reproduction steps to demonstrate the problem](url)

**JuPedSim version:** [Enter JuPedSim version here]
**OS and version:** [Enter OS name and version here]
**Compiler and version:** [Enter compiler name and version here]

**Installed Libraries:**
[Enter Boost version here]
[Enter Vtk version here]
[Enter Qt version here]

**Additional information:**

* Problem started happening recently, didn't happen in an older version of JuPedSim: [Yes/No]
* Problem can be reliably reproduced, doesn't happen randomly: [Yes/No]
* Problem happens with all files and projects, not only some files or projects: [Yes/No]
* Problem happens with the attached ini and geometry files: [Yes/No]
~~~

### Requesting features

Enhancement suggestions are tracked as issues. After you've determined which repository your enhancement suggestions is related to, create an issue on that repository and provide the following information:

 * Use a clear and descriptive title for the issue to identify the suggestion.
 * Provide a step-by-step description of the suggested enhancement in as many details as possible.
 * Provide specific examples to demonstrate the steps. Include copy/pasteable snippets which you use in those examples, as Markdown code blocks.
 * Describe the current behavior and explain which behavior you expected to see instead and why.

If you want to support us by writing the enhancement yourself consider what kind of change it is:

- **Major changes** that you wish to contribute to the project should be discussed first on our **dev mailing list** so that we can better coordinate our efforts, prevent duplication of work, and help you to craft the change so that it is successfully accepted into the project.
- **Small changes** can be crafted and submitted to our repository as a **pull or merge request**.

Nevertheless open an issue for documentation purposes with the following template:

~~~.md
[Short description of suggestion]

**Steps which explain the enhancement**

1. [First Step]
2. [Second Step]
3. [Other Steps...]

**Current and suggested behavior**

[Describe current and suggested behavior here]

**Why would the enhancement be useful to most users**

[Explain why the enhancement would be useful to most users]

[List some other text editors or applications where this enhancement exists]

**Screenshots and GIFs**

![Screenshots and GIFs which demonstrate the steps or part of JuPedSim the enhancement suggestion is related to](url)

**JuPedSim Version:** [Enter JuPedSim version here]
**OS and Version:** [Enter OS name and version here]~

~~~

## Coding Guidelines

### Format

### Unit tests

Testing should be enabled by cmake as follows:
```javascript
cmake -DBUILD_CPPUNIT_TEST=ON ..
```

After compiling (`make`) the tests can be called as follows:

```javascript
ctest
```
### Verification and validation


## Writing Documentation

### Comments
Comments have to be written in **English** everywhere. Please use markdown where applicable.

### Documenting new features
Please update the changelog with every feature/fix you provide so we can keep track of changes for new versions of JuPedSim.
