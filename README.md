# cpp_message
A barebones command line messaging app built with C++.

Download and build using the Makefile. If you want to run your own webserver, then the code is available (you'll need to change the URL within main.cpp, though). Look in the files to see how to set up the sub-directories with the correct .txt files in them.
You need to use SFML's networking library, which sadly doesn't support OpenSSL natively (and I'm too lazy to implement it myself), so it's not really too secure.
