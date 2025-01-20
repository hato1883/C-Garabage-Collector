# Projekt 2 - IOOPM

# TODO - Template bellow

# Description

## Compile Code
To compile all files run following command
```
make build 
```

## Run Code
To run the code invoke the executable using following command
```
./bin/main 
```

## Run Unit Tests
To run unit test defined in `test/` run following command
```
make unit_test 
```

## Run Memory leak check
To run memory leak check on the tests defined in `test/` run following command
```
make mem_test 
```

## Run Code Coverage
Code Line coverage is <...>% in `<...>.c` and <...>% in `<...>.c`.  
Code Branch coverage is <...>% in `<...>.c` and <...>% in `<...>.c`.
To check code coverage use the following command:
```
make coverage 
```
A html website with coverage report is generated in `./coverage/project/index.html` and it also contains the branch coverage for each file.

## Remove compiled code
To remove all compiled file use the following command:
```
make clean 
```
To remove all created objects (made through other make commands) use the following command:
```
make deepclean 
```

## Error handling
There are no error handling in the existing public functions of either webstore.c or user_interface.c
