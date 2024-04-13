# Synopsis

This project enables z/OS developers to generate a shell script or JCL from a JCL file

## Code Example

Here is an example of **jcl2sh** being used to convert PDS member IBMUSER.TEST.JCL(UPDATE) to update.sh

jcl2sh <//'IBMUSER.TEST.JCL\(UPDATE\) >./update.sh

Here is an example of **jcl2jcl** being used to convert orig.jcl to update.jcl

jcl2jcl --input=testsrc/commented.jcl

Both programs probably work either on z/OS or off. jcl2jcl was set up for 'off z/OS' and jcl2sh was set up for 'on z/OS'

## Motivation

I am not a big fan of JCL, even though I have worked on z/OS forever.
I also try to use Unix System Services as much as possible to develop software.
jcl2sh generates a 'sh' program that uses the mvscmd/mvscmdauth services for invoking MVS executables
jcl2jcl generates JCL from JCL - which can be handy if you want to 'normalize' or 'clean up' JCL.

## Installation

To install jcl2sh:

- copy the files to z/OS Unix System Services directory. For this example, we assume it is /u/ibmuser/JCL2SH
- cd to the directory (/u/ibmuser/JCL2SH)
- edit setenv.sh to point to the various MVS programs that will be tested, and to specify where your code was copied to.
- edit build.sh if required to point to your C compiler and assembler, then run the script to build the program.
- run build.sh: build.sh
- The assemble, compile and link should be 'clean' and will produce a file called 'jcl2sh'

To install jcl2jcl:

- clone this repo
- cd to the repo (JCL2SH)
- edit buildmac.sh to build for your Mac (or tweak it for another system)
- run buildmac.sh
- The assemble, compile and link should have just a few warnings and will produce a file called 'jcl2jcl'

## API Reference

To get started reading the code, begin in jcl2sh.c, which has 'main' and drives all the functions in the other files.

## Tests

To run the tests:

- cd to the directory (e.g. /u/ibmuser/JCL2SH)
- runTests.sh

This will write results to the screen as it runs the testcases in the tests sub-directory.
All tests should run clean

## Work to be done

The scanning/parsing development is well underway. Writing the generator(s) is not done yet.

## Contributors

Mike Fulton (IBM Canada) is the sole contributor at this point. I am happy to change this :)

The code still needs work. Error messages can (always) be improved and there are missing features people may need.
If anyone wants to contribute, please reach out to <fultonm@ca.ibm.com> (Mike Fulton)

## License

The code uses the Apache license
