# Code Style

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

## Tabs vs Spaces
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


## Comments 

Comments and commit messages have to be written in **English**. Please write clear and concise commit messages so that
your co-developers can directly grasp what changes on the code are you committing/pushing.

