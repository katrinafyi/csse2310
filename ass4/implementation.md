# Assignment 4 Implementation Spec

## State Structs
- A struct with associated _init and _destroy methods is called a state struct.
- An _init method MUST be accompanied by a _destroy method and vice-versa.
- Any state struct MUST be initialised to {0}.
- The initialiser SHOULD be called exactly once, on a state struct of {0}.
  - the initialiser SHOULD take enough parameters to initialise to a useful
    state.
- The destructor MUST satisfy the following properties:
  - do nothing if the given pointer is NULL or the pointed struct is {0}.
  - free all resources allocated in an initialiser.
  - do nothing if called multiple times with the same argument (idempotent)

## Comments
- Function and struct comments SHOULD specify if a parameter/return value/
  field is expected to be MALLOC'd.
