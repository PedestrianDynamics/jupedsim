Contributing to JuPedSim
========================

This project is mainly developed by a small group of researchers and students from [Jülich Research Center](http://www.fz-juelich.de/en) and [BUW](http://www.uni-wuppertal.de/).
However you are kindly invited not only to use JuPedSim but also contributing to our open-source-project.
It does not matter if you are a researcher, student or just interested in pedestrian dynamics.
There are only a few rules and advices we want to give to you:

- [Advice for new contributors](#advice-for-new-contributors)
    - [First steps](#first-steps)
    - [Guidelines](#guidelines)
    - [FAQ](#faq)

- [Reporting bugs and requesting features](#reporting-bugs-and-requesting-features)
    - [Using the issue tracker](#using-the-issue-tracker)
    - [Reporting bugs](#reporting-bugs)
    - [Requesting features](#requesting-features)

- [Writing code](#writing-code)
    - [Coding style](#coding-style)
    - Unit tests

- Writing documentation
    - Comments
    - Documenting new features

- Committing code
    - Handling pull/merge requests
    - Committing guidelines
    - Reverting commits

## Advice for new contributors

### First steps

### Guidelines

### FAQ

## Reporting bugs and requesting features
If you got a question or a problem and need support from our team feel free to contact us.
You can do this via [email](mailto:dev@jupedsim.org).

If you think you found an issue or bug in JuPedSim please use our issue tracker.

### Using the issue tracker

The issue tracker is the preferred channel for bug reports, features requests and submitting pull requests, but please respect the following restrictions:

- Please do not use the issue tracker for personal support requests. [Mail us](mailto:dev@jupedsim.org) if you need personal support.

- Please do not derail or troll issues. Keep the discussion on topic and respect the opinions of others.

If you use the issue tracker we have a list of labels you should use.

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

~~~
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

## Writing Code

### Coding style

In JuPedSim we try to code according to the *Stroustrup* style of formatting/indenting.
If you want (or have) to write code in JuPedSim you really **need** to respect that style.
This is important not just aesthetically but also practically. Diff commits are much more clearer and cleaner.

The code is formated using the automatic formatter [astyle](http://astyle.sourceforge.net/astyle.html)  with the option `--style=stroustrup`:

> Stroustrup style formatting/indenting uses stroustrup brackets.
> Brackets are broken from function definitions only.
> Brackets are attached to everything else including
> namespaces, classes, and statements within a function, arrays, structs, and enums.
> This style frequently is used with an indent of 5 spaces.

Here is an **example:**

```python
int Foo(bool isBar)
{
     if (isBar) {
          bar();
          return 1;
     } else
          return 0;
}
```

#### Tabs vs Spaces
This can be a long and religious discussion, to make it short *DO NOT* use tabs, just spaces please.

Here are some hints to configure your editor in order to use the *stroustrup* style

- **Emacs**:
    Add this to your ```.emacs```
```lisp
(setq c-default-style "stroustrup" c-basic-offset 5)
(setq indent-tabs-mode nil)
```
- **Vim**:
    Set in your config file these variables
```javascript
:set autoindent
:set cindent
:set expandtab
:set shiftwidth=5
:set softtabstop=5
```

- **Eclipse**:
    Here is a [plugin](http://astyleclipse.sourceforge.net/) for astyle in eclipse.
    Read also
    [How to change indentation width in eclipse?](https://superuser.com/questions/462221/how-do-i-reliably-change-the-indentation-width-in-eclipse)

- **Clion**
<!-- TODO -->


### Comments

Comments have to be written in **English**. Please write clear and concise commit messages so that
your co-developers can directly grasp what changes on the code are you committing/pushing.

