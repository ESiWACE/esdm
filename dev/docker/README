For the development of ESDM this directory contains all requirements to quickly
set up a development environment using docker.

For easy building Dockerfiles for different plattforms are provided in different
flavours:

	* CentOS/Fedora/RHEL like systems
	* Ubuntu/Debian like systems

# Setup

Assuming the docker service is running you can build the docker images as follows:

	cd <choose ubuntu/fedora/..>
	sudo docker build -t esdm .


After docker is done building an image, you should see the output of the 
ESDM test suite, which verifies that the development environment is set up
correctly. The output should look similar to the following output:

	Running tests...
	Test project /data/esdm/build
	    Start 1: metadata
	1/2 Test #1: metadata .........................   Passed    0.00 sec
	    Start 2: readwrite
	2/2 Test #2: readwrite ........................   Passed    0.00 sec

	100% tests passed, 0 tests failed out of 2

	Total Test time (real) =   0.01 sec


# Usage

* Running the esdm docker container will run the test suite:

	sudo docker run esdm  # should display test output


* You can also explore the development environment interactively:

	sudo docker run -it esdm bash
