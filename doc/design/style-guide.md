# Styleguide for ESDM development

This document describes the style guide to use in the code.

## General rules

  * Do not break the line after a fixed number of characters as this is the duty of the editor to use some softwrap.
    * You may use a line wrap, if that increases readability (see example below).
  * Use two characters for indentation per level
  * Documentation with Doxygen needs to be added on the header files
  * Ensure that the code does not produce WARNINGS
  * Export only the functions to the user that is needed by the user
  * The private (module-internal) interface is defined in *-internal*.h

## Naming conventions

  * use lower case for the public interface
  * functions for users provided by ESDM start with esdm_
  * auxiliary functions that are used internally start with ea_ (ESDM auxiliary) and shall be defined inside esdm-internal.h

## Example Code

struct x_t{
  int a;
  int b;
  int *p;
};

typedef struct x_t x_t; // needs always to be split separately, to allow it to coexist in a public header file

int testfunc(int a){
  {
    // Additional basic block
  }
  if (a == 5){

  }else{

  }

  // allocate variables as late as possible, such that we can see when it is needed and what it does.
  int ret;

  switch(a){
    case(5):{
      break;
    }case(2):{

    }
    default:{
      xxx
    }
  }

  return 0;
}
