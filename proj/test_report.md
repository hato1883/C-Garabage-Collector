# Test Report
Throughout the entire project, we maintained our continuous integration tools via GitHub Actions.
This provided us with information about how much of our code was tested among other things after each commit.
To streamline this process we added target to our make file to run testing checks.
These checks include unit testing, memory leaks, code coverage, style formatting.

These tests utilized the CUnit library as well as gcov to gather information about the code coverage of the project. Given this information, we could see how much of the new code was tested during each pull request.

Among these this test are a majority of them unit tests targeting the specific functions but there exist some functions that test methods such as h_gc that invokes the entire call hierarchy. in combinations with this we still have inlupp 2 tests that can test how the GC acts when memory is not enough. However, when the heap is large enough for compacting (i.e. more than one page) but too small to fit all data from inlupp 2, the program crashes. This is due to segmentation faults that come from incorrect updates of pointers during compacting in the garbarge collection.

We discovered this bug, which was not caught in the tests, right before handing in the project. This was in part because we had accidentally tested inlupp 2 with a heap so large that no compacting was performed. We have been able to create a failing test for h_gc, specifically when data is allocated at page edges.

The reason we failed to catch this bug earlier is not due to any lacking coverage. Rather, the tests covering certain branches did not catch inorrectness in the code. The analysis below was done on the tests we have handed in, before we discovered the bug.

## Code coverage for our garbage collection:

|            File           |  Total Rows  | Rows covered | Coverage Percentage  |   Missed lines     |
| ------------------------- | ------------ | ------------ | -------------------- | ------------------ |
| src/allocation.c          | 108          | 108          | 100%                 |                    |
| src/allocation_map.c      | 28           | 28           | 100%                 |                    |
| src/format_encoding.c     | 269          | 263          | 97%                  | 214,443-444,645,675,677 |
| src/gc.c                  | 244          | 242          | 99%                  | 294, 319           |
| src/gc_utils.c            | 9            | 9            | 100%                 |                    |
| src/get_header.c          | 45           | 43           | 95%                  | 127, 129           |
| src/header.c              | 23           | 23           | 100%                 |                    |
| src/is_pointer_in_alloc.c | 17           | 17           | 100%                 |                    |
| src/move_data.c           | 20           | 20           | 100%                 |                    |
| src/page_map.c            | 34           | 34           | 100%                 |                    |
| src/ptr_queue.c           | 38           | 38           | 100%                 |                    |
| src/stack.c               | 21           | 21           | 100%                 |                    |

### Following lines are uncovered due to assert guards:
- `src/format_encoding.c:214` Unreachable code due to switch statement in the default branch
- `src/format_encoding.c:645` Unreachable code due to unused encoding option
- `src/format_encoding.c:675` Unreachable code due to switch statement in the default branch
- `src/format_encoding.c:677` Unreachable code due to same as above
- `src/gc.c:294` Unreachable due to unused encoding option

### Following lines are not covered:
- `src/format_encoding.c:443-444` Code is only reached in systems that do not have 4 bytes floats/ints or 8 bytes long/double/pointer etc
- `src/gc.c:319` Code that guards against faulty headers in the heap search, hard to test.

Whilst our line coverage is overall very high we do lack some branch coverage.
We managed to cover 279 out of 318 branches giving us a overall percentage 87,7%.
Majority of the missed branches are due to assert statements with a total of 22 assert statements.
This leaves only 17 branches due to if statements. These are all due to if statements which contain multiple conditions.
Which leads to us only covering 3 out of the possible 4 branches, leaving the 4th missed.
The last branch is hard to cover due to the first condition often being dependent to the second condition.
