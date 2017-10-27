## Synopsis

This project enables z/OS developers to generate a shell script from a JCL file

## Code Example

Here is an example of **jcl2sh** being used to convert PDS member IBMUSER.TEST.JCL(UPDATE) to update.sh

jcl2sh <//'IBMUSER.TEST.JCL\(UPDATE\) >./update.sh

## Motivation

I am not a big fan of JCL, even though I have worked on z/OS forever. 
I also try to use Unix System Services as much as possible to develop software. 
jcl2sh generates a 'sh' program that uses the mvscmd/mvscmdauth services for invoking MVS executables

## Installation

To install:
- copy the files to z/OS Unix System Services directory. For this example, we assume it is /u/ibmuser/JCL2SH
- cd to the directory (/u/ibmuser/JCL2SH)
- edit setenv.sh to point to the various MVS programs that will be tested, and to specify where your code was copied to. 
- edit build.sh if required to point to your C compiler and assembler, then run the script to build the program.
- run build.sh: build.sh
- The assemble, compile and link should be 'clean' and will produce a file called 'jcl2sh'

## API Reference

To get started reading the code, begin in jcl2sh.c, which has 'main' and drives all the functions in the other files.

## Tests

To run the tests:
- cd to the directory (e.g. /u/ibmuser/JCL2SH)
- runTests.sh

This will write results to the screen as it runs the testcases in the tests sub-directory. 
All tests should run clean

## Contributors

Mike Fulton (IBM Canada) is the sole contributor at this point. I am happy to change this :)

The code still needs work. Error messages can (always) be improved and there are missing features people may need.
If anyone wants to contribute, please reach out to fultonm@ca.ibm.com (Mike Fulton)

## License

The code uses the Eclipse Public License 1.0 ( https://opensource.org/licenses/eclipse-1.0.php )
