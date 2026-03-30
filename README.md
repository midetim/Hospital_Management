This is Group 11's Hospital Management System

To run it you need to have docker desktop installed, preferably in a linux environment, but it should work on windows as well

There are two types of front ends available to use:
  The GUI front end which has a web page with easier accessibility, but more limited functionality
      or
  The CLI front end which has a terminal text based front end that has complete functionality, but is a little harder to use

In order to run the program you just need to execute run.sh in a bash shell (linux/MacOS) or in a git bash shell (Windows)

For the GUI front end run

$ ./run.sh <opt>
or
$ bash run.sh <opt>

where opt can be either cli or gui depending on what you want
example:
$ ./run.sh cli

Please note that the initial setup requires a stable internet connection, as docker will download the base image from a repo
Inital setup may take a while and the more RAM & cores you have allocated to docker will speed up the setup.

*** INITIAL SETUP COULD TAKE OVER AN HOUR ***
