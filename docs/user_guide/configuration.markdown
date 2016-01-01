# Installation
JuPedSim-modules can not be installed but compiled.

For all  modules the compilation is fairly straightforward: 

- Create a new directory and change to it
```javascript
mkdir build && cd build
```

- Run ```CMake``` to build the project

``` javascript
cmake .. 
``` 

*cmake* will check if all required  libraries are installed. 
- Compile the project
If the previous step terminated successfully run 
``` javascript
make -jN
``` 
(*N* is the number of your CPUs)

- Run a simulation
If the compilation succeeded a binary will be produced in
the direction *bin*. Run a simulation using one of
the ini files in the directory *initfiles*

For example:
```javascript
./bin/jpscore inputfiles/Bottleneck/1.1_ini-Bottleneck.xml
```

## Using an IDE

- [CLion](https://www.jetbrains.com/clion/) (Windows, Linux, Mac)

    *CLion* has build in *CMake*-support so you only have to start a new project an set the project root to your `jpscore` directory.

- [Eclipse](https://eclipse.org/) (Windows, Linux, Mac)

    Build an *Eclipse Project* with *CMake* or the *CMake-gui* and open it with *Eclipse*.

- [Visual Studio 2013](http://msdn.microsoft.com/de-de/library/dd831853.aspx) (v0.6+, Windows) 

    Run the *CMake-gui* with the generator *Visual Studio 12 (2013)* and click on the `*.sln` file to open it in Visual Studio.

    Then click on `build->build solution`.
    This should build your code.

    Current issues are known with the python test cases. Most of them are Unix specific.
    However the compilation of `jpscore` works fine. 

- XCode (v0.6+ Mac)

    Run the *CMake-gui* and generate an *XCode*-Project then open it with *XCode*.

    On Mac the standard compiler is clang/clang++.
    If the project compiles out of your console via cmake/make that does not mean it also compiles out of *XCode*.
    *XCode* forces some more strict rules while compiling.

    For instance, there was one error we could not explain:
    *XCode* seems not to know that `std::unordered_map.embrace(...)` is a valid function call
    and returns with an error message.

    However this is an *XCode*-specific problem. It does not occur if you are using e.g. `CLion`.

    Even if you compile out of console with the same compiler this throws no error. 

    You can avoid this by using the function `std::unordered_map.insert(std::make_pair(...))` instead.
