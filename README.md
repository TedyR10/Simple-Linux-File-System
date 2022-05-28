Mitran Andrei-Gabriel & Rolea Theodor-Ioan
313CA

## Project 3 - Simple Linux File System

### Description:

* This project aims to create a simple linux file system that can be accessed through bash commands.
  Brief description of the functions used:
  - createFileTree: creates the root directory and allocs memory for the structure
  - ls: lists every file in a given directory/displays the contents of a file. If ls is cast on a file, it displays its contents. Otherwise, if ls is cast on a directory, it lists every file inside of it.
  - pwd: prints the path of the current directory we are in
  - cd: switches the directory we are currently in with another one given
  - tree: prints every file and directory starting from a given node
  - mkdir: creates a new directory in the current directory
  - rmrec: deletes everything from a given directory/file
  - rm: deletes a given file
  - rmdir: deletes a given directory only if it is empty
  - touch: creates a new file in a directory
  - cp: copies a file from the current directory into another one
  - mv: moves a file from the current directory into another one
  - freeTree: frees the whole file system

### Other comments:

* In-depth comments regarding every function are used in the source code :).
  Code modularization could've been better, but we chose to stick to the given source code.
  The fact that we can use cd on a file is useful when making other functions.

### Resources:

1. [1](https://youtu.be/dQw4w9WgXcQ)
