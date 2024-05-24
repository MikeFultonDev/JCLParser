# Synopsis

This project enables z/OS developers to parse JCL and to then walk the parse tree.
The default build creates a `jcl2jcl` program that can be used to just generate JCL from JCL.
This is meant for testing and understanding of the code. A real use case would be to walk the
parse trees to do some sort of analysis.

The `altsrc` and `alttest` and `alttestsrc` directories are historical and were used for testing
the parser was working well. I am changing over to now just generate JCL from JCL so that I can 'diff' 
the output.

## Code Example

Here is an example of **jcl2jcl** being used to parse orig.jcl and print it out.

jcl2jcl --input=testsrc/commented.jcl

The code works on z/OS or off. It has been tested on z/OS and on MacOS using clang.

## Motivation

I am not a big fan of JCL, even though I have worked on z/OS forever.
I also try to use Unix System Services as much as possible to develop software.
jcl2jcl generates JCL from JCL - which can be handy if you want to 'normalize' or 'clean up' JCL.

## Installation

To install jcl2jcl:

- copy the files to z/OS Unix System Services directory. For this example, we assume it is `/u/ibmuser/JCLParser`
- cd to the directory (`/u/ibmuser/JCLParser`)
- install CMake, Ninja, GNU Make, clang if you haven't already and use the provided CMake code. 
- Alternately, just compile all the C code in `src` and link it.
- The resultant program should be called `jcl2jcl` to use the sample parser.

## Building on z/OS

- Use [Z Open Tools](https://github.com/ZOSOpenTools) to download cmake, ninja, gnu make, and install clang. 
- export CLANG_ROOT to the root directory where you installed the clang compiler.
- Generate build files if first build:
```
cd JCLParser
mkdir build
cd build
export CC=${CLANG_ROOT}/usr/lpp/IBM/oelcpp/v2r0/bin/clang
export CXX=${CLANG_ROOT}/usr/lpp/IBM/oelcpp/v2r0/bin/clang++ 
cmake ../
```
- Re-build code:
```
cmake --build .
```

This will create a jcl2jcl binary in your build directory

## Building on MacOS

- Tailor the files in the .vscode directory to suit your needs.

## API Reference

To get started reading the code, begin in `jcl2jcl.c`, which has `main` and drives all the functions in the other files.

## Tests

To Be Updated 

## Work to be done

The parser is complete as far as I know. Bug reports welcome. 
Providing interesting tools with the parser is TBD. 

## Contributors

Mike Fulton (IBM Canada) is the sole contributor at this point. I am happy to change this :)

The code still needs work. Error messages can (always) be improved and there are missing features people may need.
If anyone wants to contribute, please reach out to <fultonm@ca.ibm.com> (Mike Fulton)

## License

The code uses the Apache license
