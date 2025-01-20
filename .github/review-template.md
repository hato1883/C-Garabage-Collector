## General

### Warnings and Errors

- [ ] There are no warnings present in the project.
- [ ] There are no errors present in the project.
- [ ] There are no warnings that have been supressed in the code without proper reasoning in a comment.
- [ ] There are no warnings, errors present in compilation output.
- [ ] The entire solution can build and run locally on your machine.
- [ ] The code works.

### Logging and Debugging

- [ ] No `puts`, `printf` or similar debugging statments exists.
- [ ] All debuging code is absent.

### Working and Extensibility

- [ ] variables are provided with the smallest scope possible.
- [ ] There is no dead code (code that cannot be accessed during runtime, *don't* just rely on VS).
- [ ] Code is not repeated on duplicated (use loops instead of repitition!).
- [ ] The ideal data structures are used where appropriate (i.e; `Stack` is used instead of `List`) where applicable.
- [ ] Interfaces or abstract classes are passed as parameters to classes and methods instead of concrete implenentations.

### Readability

- [ ] The code is easy to understand.
- [ ] There is no usage of "magic numbers".
- [ ] Enumerations are used in place of constant integers where applicable.
- [ ] `for` loops have been replaced with `iterator` where speed is not a direct issue.
- [ ] Constant variables have been used where applicable.
- [ ] There are *no* complex long boolean expressions (i.e; `x = isMatched ? shouldMatch ? doesMatch ? blahBlahBlah`).
- [ ] There are *no* negatively named booleans (i.e; `notMatch`should be `isMatch` and the logical negation operator (`!`) should be used.

### Other

- [ ] All if statments have coresponding else statment, (`if (cond) {blah; blah; blah;} else {}`)
- [ ] There are unused variables.
- [ ] Arrays are checked for out of bounds conditions, just in case.
- [ ] Floating point numbers are not compared for equality, use `abs(floatA - floatB) < delta` instead
- [ ] Loops have a set length and correct termination conditions.
- [ ] Loop termination conditions are at the top of the loop prefferably inside loop condition.
- [ ] Blocks of code inside loops are as small as possible.
- [ ] No object exists longer than necessary
- [ ] Do any of the changes assume anything about input? If so, is the assumption fairly accurate or should it be omitted?

## Design

- [ ] Will developers be able to modify the program to fit changing requirements?
- [ ] Is there hard-coded data that should be modifiable?
- [ ] Is the program sufficiently modular? Will modifications to one part of the program require modifications to others?
- [ ] Do you, the reviewer, understand what the code does?
- [ ] Is all of the functionality necessary? Could parts be removed without changing the performance?
- [ ] If code is commented, could the code be rewritten so that the comments aren't necessary?

## Styling and Coding Conventions

- [ ] Any new files have been named consistently and spelt correctly.
- [ ] Any and all variables have been named simply and if possible, short and to the point.
- [ ] There is _no_ commented out code.
- [ ] All changes follow the styling and coding conventions of the repository.

## Documentation

- [ ] The entire interface has been clearly documented in header (`.h`) file.
- [ ] Any changes made in the code has been reflected in the documentation.
- [ ] All edges cases are described in documentation (i.e; what if I pass `null` to _x_?)
- [ ] Data structures and units of measurement are clearly documented.
- [ ] In hard-to-understand areas comments exist and describe rationale or reasons for decisions in code.
- [ ] Where applicable, remarks and source code examples have been provided to _at least_ the _public_ interface (note that if this is a PR to be merged into a release, almost *all* code should provide an example).
- [ ] Is the documentation accurate?
- [ ] Is the documentation easy to understand?

## Testing

- [ ] There are unit tests provided to accommodate the changes.
- [ ] The provided unit tests have a code coverage of _at least_ 80%.
- [ ] The tests follow the correct styling (MethodName_Should_ExpectedBehaviour_When_StateUnderTest)
- [ ] The tests follow the [AAA (Arrange, act and assert) methodology](https://docs.telerik.com/devtools/justmock/basic-usage/arrange-act-assert) and are clearly commented.
- [ ] All tests pass locally on your machine
- [ ] All tests pass on CI.

## Exceptions & Error Handling

- [ ] Constructors, methods, etc do not accept `null` (unless otherwise stated so in the documentation and PR with relevant context)
- [ ] Disposable resources are properly closed even when an exception occurs in using them (free any used memory that is incomplete).
- [ ] No printing of exception in library functions, let UI handel it.
- [ ] Are the error messages / codes, if any, informative?
- [ ] When an error or exception occurs, does the user get adequate information about what he or she did wrong?

## Security

- [ ] All personal data inputs are checked (for the correct type, length/size, format, and range).
- [ ] Invalid parameter values are handled such that exceptions are not thrown (i.e; don't throw an exception if the user gives the wrong email and password combination).