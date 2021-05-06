Includes generator for C++ types.

You no longer need to remember in which header file the required type is declared. Now it is enough to write:

  #include <std/back_inserter>
  
And the required header will be automatically included.

For C++ std types library look to https://github.com/al-martyn1/std_headers

For Qt types library look to https://github.com/al-martyn1/qt_headers


To build umba-make-headers executable, create the 'cxx.bat' file which calls C++ compiler.

To automatically deploy umba-make-headers executable into Your system, create the 'deploy.bat' file
in this directory.

Add to the 'deploy.bat' file actions for deploying umba-make-headers executable into Your system.