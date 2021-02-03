# tiny-webserver:

Small web server implemented as described in:

"Computer Systems: A Programmer's Perspective"\
Randal E. Bryant, David R. O'Hallaron\
Carnegie Mellon University

## Prerequisites:

-runnable on linux distributions only\
-to build the executables, run "make" in the repository root or follow the steps taken in "Makefile"

IMPORTANT: set compiler and flags in "Makefile" according to your setup.
 
## Functionalities:

-Operations implemented:\
    ```GET <path-to-resource> <http-ver>```
    
IMPORTANT:\
-there are different functions called when serving dynamic content (resources stored under cgi-bin) or static content (basically the rest)\
-static content simply gets retrieved\
-dynamic resource handling has to be implemented for each resource that is stored under cgi-bin
