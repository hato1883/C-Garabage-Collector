
###
# Constant declarations
###

# Directory structure
DEPDIR			:=dep
BINDIR			:=bin
OBJDIR			:=obj
SRCDIR			:=src
LIBDIR			:=lib
TESTDIR			:=test
COVDIR			:=coverage
PROFDIR			:=profiling
LOGDIR			:=logs
DEMODIR			:=demo

# C Compiler
CC     				:=gcc
# Compiler Flags
CFLAGS 				:=-Wall -Wextra -pedantic -g
# Compiler Linker Flags
LDFLAGS 			:=-lm
# C Unit Dependency / link
CUNIT_LINK     		:=-lcunit
# Compiler Coverage Flags
CCOVFLAGS 			:=-O0 --coverage
# Compiler Profiling Flags
CPROFFLAGS 			:=-O3 -pg
# Error testing linking flags
MEMORY_WRAP 		:=-Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=h_alloc_struct -Wl,--wrap=h_alloc_raw -Wl,--wrap=free
IO_WRAP 			:=-Wl,--wrap=printf -Wl,--wrap=puts -Wl,--wrap=putc -Wl,--wrap=getchar
SRC_COV_PERC_PASS 	:=100
SRC_COV_PERC_WARN 	:=90

# Formatter Compiler
FF     				:=clang-format-18
# Compiler Flags
FFLAGS-CHECK 		:=--style=file --Werror --Wclang-format-violations -n
# Compiler Flags
FFLAGS-FORMAT 		:=--style=file -i


# Memory leak Test runner
MEMLEAK_RUNNER		:= valgrind
MEMLEAK_PRE_FLAGS	:= --leak-check=full --error-exitcode=10 -s
MEMLEAK_POST_FLAGS	:= > /dev/null

# Heap Profiling
HEAP_RUNNER			:= valgrind
HEAP_PRE_FLAGS		:= --tool=massif --heap=yes --stacks=yes --threshold=0.01
HEAP_POST_FLAGS		:= > /dev/null 2>&1

# Cache Profiling
CACHE_RUNNER		:= valgrind
CACHE_PRE_FLAGS		:= --tool=cachegrind --cache-sim=yes --branch-sim=yes
CACHE_POST_FLAGS	:= > /dev/null 2>&1

# Find all .c files in our project
# Both in test/ and src/

EXES				:= main freq_count large-heap-gc small-heap-gc large-heap-malloc small-heap-malloc lists-gc lists-gc-compact
EXES				:= $(EXES) user_interface_test webstore_test webstore_run_test
EXES				:= $(EXES) fifo_queue_test fifo_queue_error_test hash_table_error_test hash_table_test iterator_error_test iterator_test linked_list_error_test linked_list_test 
EXES				:= $(EXES) get_header_test is_pointer_in_alloc_test ptr_queue_test allocation_test move_data_test find_pointer_in_alloc_test compacting_test allocation_map_test create_header_test encoding_enum_test format_encoding_test gc_test page_map_test stack_test
EXES				:= $(EXES) gc_regression_test

MOCK				:= ui_mocking oom

SRCS				:= ${shell find ./ -name '*.c' | xargs -n1 basename}# Find all .c files
DEPS				:= ${addprefix $(DEPDIR)/,${SRCS:.c=.d}}			# Create dependency files for each c file
SRCS				:= ${filter-out ${addsuffix .c,$(EXES)}, $(SRCS)}	# Remove exes
SRCS				:= ${filter-out ${addsuffix .c,$(MOCK)}, $(SRCS)}	# Remove mocking libraries

MOCK_OBJS			:= ${addprefix $(DEPDIR)/,${MOCK:=.o}}

OBJS				:= ${addprefix $(OBJDIR)/,${SRCS:.c=.o}}
OBJS_COV			:= ${OBJS:.o=.coverage.o}
OBJS_PROF			:= ${OBJS:.o=.profiling.o}

# Make sure that any objects and dependencies that are created do not get deleted after target is run.
.PRECIOUS: $(OBJS) $(OBJS_COV) $(OBJS_PROF) obj/%.o obj/%.coverage.o

default: demo
	./bin/main < $(DEMODIR)/sample-input.txt

demo: $(BINDIR)/main

build: ${addprefix $(BINDIR)/,$(EXES)}

clean:
	@rm -rf ./$(BINDIR)/ ./$(OBJDIR)/ ./$(DEPDIR)/ ./$(COVDIR)/ ./$(LOGDIR)/ ./$(PROFDIR)/
	@rm -f gmon.out gmon.sum

deepclean: clean
	@rm -rf ./$(PROFDIR)/ ./$(LOGDIR)/

installdirs: | $(COVDIR)/ $(BINDIR)/ $(OBJDIR)/ $(DEPDIR)/

all:
	@echo "Executables found: ${EXES}"
	@echo "Sources found: ${SRCS}"
	@echo "Objects found: ${OBJS}"
	@echo "Dependencies found: ${DEPS}"


.PHONY: full_test
full_test: test memtest

#######################################################
#                                                     #
#      Target to create colored unit test report      #
#                                                     #
#######################################################

# All files to execute to test code coverage
# By default this will be all files ending in _test.c
TEST_MANUAL		:= ${shell find ./ -name '*_test.c'}

# DO NOT EDIT VARIABLE BELLOW
# add files in MANUAL variable above
TEST_BINARIES	:= ${addprefix $(BINDIR)/,${basename ${shell echo $(TEST_MANUAL) | xargs -n1 basename}}}

.PHONY: test
.ONESHELL:
test: $(TEST_BINARIES) | ${LOGDIR}/
# Escape first @ sign when using oneshell with comment
	@echo ""
#	Set default exit code to 0
	SHOULD_PASS=0
	@echo ""
	@echo "Running tests"
	@echo ""

	for v in $(TEST_BINARIES)
	do
		NAME=$$( echo $$v | xargs -n1 basename)

		./$${v} > ${LOGDIR}/$${NAME}_unit_test_log.txt
		CRASHED=$$?
		if [ $$CRASHED -ne 0 ]
		then
#			Update SHOULD_PASS variable to inform of crash
			echo "\033[1;31m"$${v} "Failed due to errors or crash!" "\033[0;37m"
			echo ""
			SHOULD_PASS=1; \
		fi

		
		awk '
			BEGIN {
				err = 0;
				line_above_failed = 0;
				reached_tests = 0;
				is_at_summery = 0;
				cmd="cat "ARGV[1]" | grep -oP \"\\s*asserts\\s+([0-9]+\\s+){3}\\K[0-9]+\""
				cmd | getline err
				close(cmd)
				cmd="cat "ARGV[1]" | grep -oP \".*\\.$$\""
				cmd | getline crash
				close(cmd)
				print crash;
			} {
				if ($$0 ~ /Suite:.*/) {
					print $$0;
				}

				if ($$0 ~ /.*Run Summary.*/) {
					is_at_summery = 1;
				}

				if ($$0 ~ /.*Test:.*/) {
					reached_tests = 1;
				}

				if (err > 0) {
					if (line_above_failed == 1) {
						print "\033[1;31m" $$0 "\033[0;37m";
						line_above_failed = 0;
					} else if ($$0 ~ /.*passed/) {
						print "\033[2;30m" $$0 "\033[0;37m";
					} else if ($$0 ~ /.*FAILED/) {
						line_above_failed = 1;
						print "\033[1;31m" $$0 "\033[0;37m";
					} else {
						if (reached_tests == 1 && is_at_summery == 0 && line_above_failed == 0) {
							print $$0;
						}
					}
				} else if (length(crash) != 0) {
					print "\033[1;31m" $$0 "\033[0;37m";
				} else {
					if ($$0 ~ /.*passed/) {
					} else if ($$0 ~ /.*FAILED/) {
					} else {
					};
				}

				if (is_at_summery == 1) {
					if (err > 0) {
						print "\033[1;31m" $$0 "\033[0;37m";
					} else {
						print $$0;
					}
				}
				
			} END {
				exit err + length(crash);
			}
		' ${LOGDIR}/$${NAME}_unit_test_log.txt
		EXIT_CODE=$$?

		if [ $$EXIT_CODE -ne 0 ]
		then
#			Update SHOULD_PASS variable to inform of crash
			SHOULD_PASS=1; \
		else
#			Remove temporary log file, user will not read it anyways
			rm -f logs/$${NAME}_unit_test_log.txt
#			Inform of of passing test
			echo "\033[1;32mUnit test Passed for $$v" "\033[0;37m"
		fi

		echo "";  \
    done
	

	if [ $$SHOULD_PASS -ne 0 ]
	then
#			Update SHOULD_PASS variable to inform of crash
		echo "\033[1;31m###" "\033[0;37m"
		echo "\033[1;31m# One or more Unit Test Suites has failed!" "\033[0;37m"
		echo "\033[1;31m###" "\033[0;37m"
	else
#		rm -rf logs/
#			Inform of of passing test
		echo "\033[1;32m###" "\033[0;37m"
		echo "\033[1;32m# All Test Suites passed all their tests" "\033[0;37m"
		echo "\033[1;32m###" "\033[0;37m"
	fi
	@echo ""

#	If one of source or test coverage fail then exit make command with non 0 exit code
	if [ $$SHOULD_PASS -eq 1 ]
	then
		exit 2
	fi
#


############################################
#                                          #
#      Target to create memory report      #
#                                          #
############################################

.PHONY: memtest
.ONESHELL:
memtest: ${TEST_BINARIES} | ${LOGDIR}/
	@echo
	HAS_LEAK=0
	for v in $(TEST_BINARIES)
	do
#		Fancy printing of status
		echo "#" "\033[1;30m" "Running Memory Leak test using valgrind on:" "\033[0;37m"
		echo "#" "\033[1;30m" "    $${v}..." "\033[0;37m"
		NAME=$$( echo $$v | xargs -n1 basename)
#		Run Memory leak testing program
		$(MEMLEAK_RUNNER) $(MEMLEAK_PRE_FLAGS) $$v $(MEMLEAK_POST_FLAGS) 2> ${LOGDIR}/$${NAME}_log.txt
#		Capture exit code
		EXIT_CODE=$$?
		if [ $$EXIT_CODE -eq 10 ]
		then
#			Memory leak found Set output to bold red
			echo "\033[2;31m"
#			Print log of found leak
			echo "exit code for test was: $$EXIT_CODE"
			cat ${LOGDIR}/$${NAME}_log.txt
#			Reset output to Normal White
			echo "\033[0;37m"
#			Inform of error with message
			echo "#" "\033[2;31m" "ERROR: $$v has encountered Memory Leaks" "\033[0;37m"
#			Update HAS_LEAK variable to inform of crash
			HAS_LEAK=1; \
		else
#			Remove temporary log file, user will not read it anyways
			rm -f ./${LOGDIR}/$${NAME}_log.txt
#			Inform of of passing test
			echo "#" "\033[2;32m" "Memory Leak Passed for $$v" "\033[0;37m"
		fi
		echo;  \
    done
#	If HAS_LEAK is set to 1, exit with non 0 code
	if [ $$HAS_LEAK -eq 1 ]
	then
		exit $$HAS_LEAK
	fi
#




############################################
#                                          #
#       Target to check formatting         #
#                                          #
############################################
SOURCE_FILES := ${shell find . -type f -regex ".*\.[ch]?$$"}
.PHONY: check_format
.ONESHELL:
check_format:
	@echo
	NEEDS_FORMATTING=0
	echo "#" "\033[1;30m" "Running Formatter on:" "\033[0;37m"
	for v in $(SOURCE_FILES)
	do
#		Fancy printing of status
		echo -n "#" "\033[1;30m" "    $${v}..." "\033[0;37m"
#		Run Memory leak testing program
		$(FF) $(FFLAGS-CHECK) $$v 2> /dev/null
#> /dev/null
#		Capture exit code
		EXIT_CODE=$$?
		if [ $$EXIT_CODE -ne 0 ]
		then
#			Unformatted file found
#			Red color
#			Print log of violating file name
			echo "\033[1;31m" "Needs formatting" "\033[0;37m"
#			Reset output to Normal White
#			Update NEEDS_FORMATTING variable to inform of crash
			NEEDS_FORMATTING=1;\
		else
			echo "\033[1;32m" "Passed" "\033[0;37m"
		fi;  \
    done
#	If NEEDS_FORMATTING is set to 1, exit with non 0 code
	if [ $$NEEDS_FORMATTING -eq 1 ]
	then
		exit $$NEEDS_FORMATTING
	fi
#
.PHONY: format
.ONESHELL:
format:
	@echo
	echo "#" "\033[1;30m" "Running Formatter on:" "\033[0;37m"
	for v in $(SOURCE_FILES)
	do
#		Fancy printing of status
		echo -n "#" "\033[1;30m" "    $${v}..." "\033[0;37m"
#		Run Memory leak testing program
		$(FF) $(FFLAGS-FORMAT) $$v
		echo "\033[1;32m" "Formatted" "\033[0;37m"; \
	done
#






#######################################################
#                                                     #
# Target to create colored coverage report (and html) #
#                                                     #
#######################################################

# All files to execute to test code coverage
# By default this will be all files ending in _test.c
COVERAGE_MANUAL		:= ${shell find ./ -name '*_test.c'}

# DO NOT EDIT VARIABLE BELLOW
# add files in MANUAL variable above
COVERAGE_BINARIES	:= ${addprefix $(BINDIR)/,${basename ${shell echo $(COVERAGE_MANUAL) | xargs -n1 basename}}}

.PHONY: coverage
.ONESHELL:
coverage: $(COVERAGE_BINARIES) | $(COVDIR)/
# Escape first @ sign when using oneshell with comment
	@echo ""
#	Set default exit code to 0
	SHOULD_PASS=0
	@echo ""
	@echo "Getting Coverage..."
	@echo ""

#	Reset previous coverage tracking if such a thing exists
	lcov --directory $(OBJDIR)/ --zerocounters -q

#	Run each coverage contributing file once
#	$(foreach var,$(COVERAGE_BINARIES),./$(var) > /dev/null;)
	for v in $(COVERAGE_BINARIES)
	do
		echo "Collecting Coverage from $${v}"
		./$${v} > /dev/null; \
    done

	@echo ""
	@echo "Generating report..."
	@echo ""
#	Generate lcov info file from data
	lcov -q --rc lcov_branch_coverage=1 --base-directory . --directory $(OBJDIR)/ -c -o $(COVDIR)/project.info  2> /dev/null
#	Generate html from lcov file
	genhtml -q --branch-coverage -o $(COVDIR)/project -t "project test coverage" --num-spaces 4 $(COVDIR)/project.info


#
#	Code Coverage for files in lib/
#	
	LIB_COVERAGE=$$(gcovr -r $(LIBDIR)/ $(OBJDIR)/ --txt 2> /dev/null)
#	Print colored version of lib files coverage output
	echo "$$LIB_COVERAGE" | \
	awk 'BEGIN {lowest_coverage = 100}{
		if ($$1 ~ /.*\.c/) {
			if(NR > 6 && $$4+0 >= $(SRC_COV_PERC_PASS)) {
				print "\033[2;32m" $$0 "\033[0;37m";
				lowest_coverage = $$4+0 < lowest_coverage ? $$4+0 : lowest_coverage
			} else if($$4+0 >= $(SRC_COV_PERC_WARN)) {
				print "\033[33m" $$0 "\033[0;37m"
				lowest_coverage = $$4+0 < lowest_coverage ? $$4+0 : lowest_coverage
			} else {
				print "\033[1;31m" $$0 "\033[0;37m"
				lowest_coverage = $$4+0 < lowest_coverage ? $$4+0 : lowest_coverage
			}
		} else if ($$1 ~ /TOTAL/) {
			if($$4+0 >= $(SRC_COV_PERC_PASS)) {
				print "\033[1;32m" $$0 "\033[0;37m";
			} else if($$4+0 >= $(SRC_COV_PERC_WARN)) {
				print "\033[1;33m" $$0 "\033[0;37m"
			} else {
				print "\033[1;31m" $$0 "\033[0;37m"
			}
		} else {
			print $$0;
		}
	} END {
		exit lowest_coverage;
	}'
	LIB_PERCENT=$$?

#	Print information regarding if coverage of library files are good or not
	if [ $$LIB_PERCENT -lt $(SRC_COV_PERC_PASS) ] && [ $$LIB_PERCENT -ge $(SRC_COV_PERC_WARN) ]
	then
		echo "\033[1;33m""###""\033[0;37m"
		echo "\033[1;33m""# Warrning: One or more library code files contains small amount unused code, Marking as Passed" "\033[0;37m"
		echo "\033[1;33m""###""\033[0;37m"
	elif [ $$LIB_PERCENT -lt $(SRC_COV_PERC_WARN) ]
	then
		echo "\033[1;31m""###""\033[0;37m"
		echo "\033[1;31m""# ERROR: One or more library code files contains LARGE amount unused code. Marking as failed" "\033[0;37m"
		echo "\033[1;31m""###""\033[0;37m"
		SHOULD_PASS=1
	fi

#
#	Code Coverage for files in test/ and demo/test/
#	
#	Generate report summary to report in terminal
	TEST_COVERAGE=$$(gcovr $(TESTDIR)/ $(DEMODIR)/$(TESTDIR)/ $(OBJDIR)/ -e $(LIBDIR)/ -e $(SRCDIR)/ -e $(DEMODIR)/$(SRCDIR)/ -e $(DEMODIR)/$(TESTDIR)/oom.c -e $(DEMODIR)/$(TESTDIR)/ui_mocking.c --txt  2> /dev/null)

#	Print colored version of test files coverage output
	echo "$$TEST_COVERAGE" | \
	awk 'BEGIN {err = 0} {
		if ($$1 ~ /.*\.c/) {
			if(NR > 6 && $$2 - $$3 <= 5) {
				print "\033[2;32m" $$0 "\033[0;37m";
			} else if($$4+0 >= 90) {
				print "\033[33m" $$0 "\033[0;37m";
				err = err < 1 ? 1 : err
			} else {
				print "\033[1;31m" $$0 "\033[0;37m";
				err = 2
			}
		} else if ($$1 ~ /TOTAL/) {
			print "\033[1;30m""Note: The 5 uncovered lines on each file is due to test suite crash handling""\033[0;37m";
			print "------------------------------------------------------------------------------";
			if(($$2 - $$3) / (NR-8) <= 5) {
				print "\033[1;32m" $$0 "\033[0;37m";
			} else if($$4+0 >= 90) {
				print "\033[1;33m" $$0 "\033[0;37m";
			} else {
				print "\033[1;31m" $$0 "\033[0;37m";
			}
		} else {
			print $$0;
		}
	} END {
		exit err;
	}'
	TEST_STATUS=$$?
#	Print CI status
#	Print information regarding if coverage of cunit test files are good or not
	if [ $$TEST_STATUS -eq 1 ]
	then
		echo "\033[1;33m""Warning: Test contains small amount unused code, Marking as Passed" "\033[0;37m"
	elif [ $$TEST_STATUS -eq 2 ]
	then
		echo "\033[1;31m""###""\033[0;37m"
		echo "\033[1;31m""# ERROR: One or more test contains LARGE amount unused code. Marking as failed" "\033[0;37m"
		echo "\033[1;31m""###""\033[0;37m"
		SHOULD_PASS=1
	fi
	@echo ""

#
#	Code Coverage for files in src/ and demo/src/
#	
	SRC_COVERAGE=$$(gcovr $(SRCDIR)/ $(DEMODIR)/$(SRCDIR)/ $(OBJDIR)/ -e $(LIBDIR)/ -e $(TESTDIR)/ -e $(DEMODIR)/$(TESTDIR)/ --txt 2> /dev/null)
#	Print colored version of src files coverage output
	echo "$$SRC_COVERAGE" | \
	awk 'BEGIN {lowest_coverage = 100}{
		if ($$1 ~ /.*\.c/) {
			if(NR > 6 && $$4+0 >= $(SRC_COV_PERC_PASS)) {
				print "\033[2;32m" $$0 "\033[0;37m";
				lowest_coverage = $$4+0 < lowest_coverage ? $$4+0 : lowest_coverage
			} else if($$4+0 >= $(SRC_COV_PERC_WARN)) {
				print "\033[33m" $$0 "\033[0;37m"
				lowest_coverage = $$4+0 < lowest_coverage ? $$4+0 : lowest_coverage
			} else {
				print "\033[1;31m" $$0 "\033[0;37m"
				lowest_coverage = $$4+0 < lowest_coverage ? $$4+0 : lowest_coverage
			}
		} else if ($$1 ~ /TOTAL/) {
			if($$4+0 >= $(SRC_COV_PERC_PASS)) {
				print "\033[1;32m" $$0 "\033[0;37m";
			} else if($$4+0 >= $(SRC_COV_PERC_WARN)) {
				print "\033[1;33m" $$0 "\033[0;37m"
			} else {
				print "\033[1;31m" $$0 "\033[0;37m"
			}
		} else {
			print $$0;
		}
	} END {
		exit lowest_coverage;
	}'
	SRC_PERCENT=$$?

#	Print information regarding if coverage of source files are good or not
	if [ $$SRC_PERCENT -lt $(SRC_COV_PERC_PASS) ] && [ $$SRC_PERCENT -ge $(SRC_COV_PERC_WARN) ]
	then
		echo "\033[1;33m""###""\033[0;37m"
		echo "\033[1;33m""# Warning: One or more source code files contains small amount unused code, Marking as Passed" "\033[0;37m"
		echo "\033[1;33m""###""\033[0;37m"
	elif [ $$SRC_PERCENT -lt $(SRC_COV_PERC_WARN) ]
	then
		echo "\033[1;31m""###""\033[0;37m"
		echo "\033[1;31m""# ERROR: One or more source code files contains LARGE amount unused code. Marking as failed" "\033[0;37m"
		echo "\033[1;31m""###""\033[0;37m"
		SHOULD_PASS=1
	fi

	@echo ""
	@echo "Code Coverage Analysis Completed"
	@echo ""

#	If one of source or test coverage fail then exit make command with non 0 exit code
	if [ $$SHOULD_PASS -eq 1 ]
	then
		exit 2
	fi
#


##################################################
#                                                #
# Target to run profiling tools on chosen files  #
#                                                #
##################################################

# All files to execute to generate a profile of program
PROFILE_MANUAL			?= main_profiling

# DO NOT EDIT VARIABLE BELLOW
# add files in MANUAL variable above
PROFILE_BINARIES		:= ${addprefix $(BINDIR)/,${basename ${shell echo $(PROFILE_MANUAL) | xargs -n1 basename}}}
PROFILE_BIN_INPUT_TEXT	?= demo/profile-input-add-stock.txt demo/profile-input-add-to-carts.txt demo/profile-input-quit.txt
PROFILE_BIN_ARGS		?= ""

.PHONY: profile_all
profile_all: profile_loops profile_lists profile_progs

.PHONY: profile_progs
profile_progs:
	$(MAKE) profile
	$(MAKE) profile PROFILE_MANUAL=freq_count_profiling PROFILE_BIN_ARGS=demo/bible.txt PROFILE_BIN_INPUT_TEXT=""

.PHONY: profile_loops
profile_loops:
	@$(MAKE) profile PROFILE_MANUAL=large-heap-gc_profiling PROFILE_BIN_ARGS=a PROFILE_BIN_INPUT_TEXT=""
	@$(MAKE) profile PROFILE_MANUAL=large-heap-malloc_profiling PROFILE_BIN_ARGS=a PROFILE_BIN_INPUT_TEXT=""
	@$(MAKE) profile PROFILE_MANUAL=small-heap-gc_profiling PROFILE_BIN_ARGS=a PROFILE_BIN_INPUT_TEXT=""
	@$(MAKE) profile PROFILE_MANUAL=small-heap-malloc_profiling PROFILE_BIN_ARGS=a PROFILE_BIN_INPUT_TEXT=""

.PHONY: profile_lists
profile_lists:
	m=20000; n=1000; while [ $$n -le $$m ] ; do \
		echo "n set to $$n, m set to $$m"; \
		$(MAKE) alt_profile PROFILE_MANUAL=lists-gc_profiling PROFILE_BIN_ARGS=a PROFILE_BIN_INPUT_TEXT="" N=$$n M=$$m; \
		$(MAKE) alt_profile PROFILE_MANUAL=lists-gc-compact_profiling PROFILE_BIN_ARGS=a PROFILE_BIN_INPUT_TEXT="" N=$$n M=$$m; \
        n=$$(($$n+1000)); \
	done

.PHONY: profile
.ONESHELL:
profile: $(PROFILE_BINARIES) $(patsubst %_profiling,%,$(PROFILE_BINARIES)) | $(PROFDIR)/ 
# Escape first @ sign when using oneshell with comment
	@echo ""
	@echo ""
	@echo "PROFILE_BINARIES == $(PROFILE_BINARIES)"
	@echo "PROFILE_BIN_INPUT_TEXT == $(PROFILE_BIN_INPUT_TEXT)"
	@echo "PROFILE_BIN_ARGS == $(PROFILE_BIN_ARGS)"
	@echo "Collecting runtime for all given files"
	@echo ""

#	Save short commit sha in variable, used for naming current commits profiling results.
	GIT_COMMIT_ID=$$(git rev-parse --short HEAD)
	@mkdir -p $(PROFDIR)/$${GIT_COMMIT_ID}/
	@mkdir -p $(PROFDIR)/$${GIT_COMMIT_ID}/data

#	Run each profiling contributing file once
	for v in $(PROFILE_BINARIES)
	do
		echo "v is set to: $${v}"
#		Get filename without path
		NAME=$$( echo $$v | xargs -n1 basename)
#		Get normal compilation of file without -pg flag
		NON_PG=$$( echo "$${v}" | sed 's/_profiling//g' )
		echo "NOT PROFILING binary: $${NON_PG}"
		mkdir -p $(PROFDIR)/$${GIT_COMMIT_ID}/$${NAME}/
#		Set how many times to run a file to reduce jitter in results
		count=20
#		For each time create a file with format profiling/<executable-file-name>_result_index
#		where index is loop step that is left zero padded to always be 3 digits long
		echo "Collecting Runtime profile for $$v"
		for i in $$(seq $$count)
		do
			echo "Executing: $${v}, Remaining executions: $${i}/$${count}..."
			INDEX=$$(printf %03d $${i})
#			run time command with verbose flag to get all info and save in file mentioned above
ifeq ($(PROFILE_BIN_ARGS),"")
			RESULT=$$( ( ( cat ${PROFILE_BIN_INPUT_TEXT} | command time -v $$v; ) 1>/dev/null; ) 2>$(PROFDIR)/$${GIT_COMMIT_ID}/$${NAME}/$${NAME}_result_$${INDEX}; )
else
			RESULT=$$( ( ( command time -v $$v ${PROFILE_BIN_ARGS}; ) 1>/dev/null; ) 2>$(PROFDIR)/$${GIT_COMMIT_ID}/$${NAME}/$${NAME}_result_$${INDEX}; )
endif

#			if it is the first loop then move gmon.out to gmon.sum
			if [ $$i -eq 1 ]
			then
				mv gmon.out gmon.sum
			else
#				Otherwise merge new gmon.out into gmon.sum
				gprof -s $$v gmon.out gmon.sum
			fi
		done
		echo ""

#		Save all temporary result files into a variable
		TMP_RESULT_FILES=$$(find ./ -name "$${NAME}_result*")
#		for each file check the second line
#		if User time (seconds): contains a lower number than we have stored
#		update our runtime var and set file by dividing global line number by 23 due to each file being 23 lines long and casting to int
#		when we reach the end we print the id of the file with best User time (seconds): result.
		BEST_TIME=$$(echo $${TMP_RESULT_FILES} | xargs awk 'BEGIN{runtime = -1; file = 1;}{
			if (FNR == 2) {
				if (runtime == -1) {
					runtime = $$4+0;
				} else if (runtime > $$4+0) {
					runtime = $$4+0;
					file = int(NR / 23) + 1;
				}
			}
		} END {print sprintf("%03d", file);}')

#		rename the temporary file with best result into <Commit-sha>_<executable-name>_runtime
		mv $(PROFDIR)/$${GIT_COMMIT_ID}/$${NAME}/$${NAME}_result_$${BEST_TIME} $(PROFDIR)/$${GIT_COMMIT_ID}/$${NAME}/$${NAME}_runtime
#		Remove all other temporary files as they will not be needed
		rm -rf $${TMP_RESULT_FILES}
#		create report using the gmon.sum and save into <Commit-sha>_<executable-name>_function_usage_analysis
		gprof -b $$v gmon.sum > $(PROFDIR)/$${GIT_COMMIT_ID}/$${NAME}/$${NAME}_function_usage_analysis


#		Get Heap profile		
		echo ""
		echo "Getting heap profile from $${NON_PG}"
ifeq ($(PROFILE_BIN_ARGS),"")
		cat ${PROFILE_BIN_INPUT_TEXT} > tmp.txt
		$(HEAP_RUNNER) $(HEAP_PRE_FLAGS) $${NON_PG} < tmp.txt $(HEAP_POST_FLAGS)
		rm tmp.txt
else
		$(HEAP_RUNNER) $(HEAP_PRE_FLAGS) $${NON_PG} ${PROFILE_BIN_ARGS} $(HEAP_POST_FLAGS)
endif
		HEAP_OUTPUT=$$(find . -name "massif.out.*" | tail -1 | grep -o -e "[[:digit:]]\{1,\}")
		mv massif.out.$${HEAP_OUTPUT} $(PROFDIR)/$${GIT_COMMIT_ID}/data/$${NAME}_massif.data
		ms_print --threshold=0.01 "$(PROFDIR)/$${GIT_COMMIT_ID}/data/$${NAME}_massif.data"  > $(PROFDIR)/$${GIT_COMMIT_ID}/$${NAME}/$${NAME}_heap_profile
		echo "Heap profile extracted from $${NON_PG}"
		echo ""
		

#		Get Cache profile
		echo ""
		echo "Getting cache profile from $${NON_PG}"
ifeq ($(PROFILE_BIN_ARGS),"")
		cat ${PROFILE_BIN_INPUT_TEXT} > tmp.txt
		$(CACHE_RUNNER) $(CACHE_PRE_FLAGS) $${NON_PG} < tmp.txt > $(PROFDIR)/$${GIT_COMMIT_ID}/$${NAME}/$${NAME}_cachegrind_output.txt 2>&1
		rm tmp.txt
else
		$(CACHE_RUNNER) $(CACHE_PRE_FLAGS) $${NON_PG} ${PROFILE_BIN_ARGS} > $(PROFDIR)/$${GIT_COMMIT_ID}/$${NAME}/$${NAME}_cachegrind_output.txt 2>&1
endif
		CACHE_OUTPUT=$$(find . -name "cachegrind.out.*" | tail -1 | grep -o -e "[[:digit:]]\{1,\}")
		mv cachegrind.out.$${CACHE_OUTPUT} $(PROFDIR)/$${GIT_COMMIT_ID}/data/$${NAME}_cachegrind.data
		cg_annotate "$(PROFDIR)/$${GIT_COMMIT_ID}/data/$${NAME}_cachegrind.data" > $(PROFDIR)/$${GIT_COMMIT_ID}/$${NAME}/$${NAME}_cache_profile
		echo "Cache profile extracted from $${NON_PG}"
		echo ""; \
    done

	@echo "###"
	@echo "# Code Profiling Completed"
	@echo "# "
	@echo "# Results can be found in $(PROFDIR)/$${GIT_COMMIT_ID}/"
	@echo "###"


# Alternative profiling script to bulk profile.
# set M and N to your values, and they will be passed to target program
M ?= 10000
N ?= 1000
.PHONY: alt_profile
.ONESHELL:
alt_profile: $(PROFILE_BINARIES) $(patsubst %_profiling,%,$(PROFILE_BINARIES)) | $(PROFDIR)/ 
# Escape first @ sign when using oneshell with comment
	@echo ""
	@echo ""
	@echo "PROFILE_BINARIES == $(PROFILE_BINARIES)"
	@echo "PROFILE_BIN_INPUT_TEXT == $(PROFILE_BIN_INPUT_TEXT)"
	@echo "PROFILE_BIN_ARGS == $(PROFILE_BIN_ARGS)"
	@echo "Collecting runtime for all given files"
	@echo ""

#	Save short commit sha in variable, used for naming current commits profiling results.
	GIT_COMMIT_ID=$$(git rev-parse --short HEAD)
	@mkdir -p $(PROFDIR)/$${GIT_COMMIT_ID}/
	@mkdir -p $(PROFDIR)/$${GIT_COMMIT_ID}/data
	@mkdir -p $(PROFDIR)/lists/M$(M)/N$(N)/

#	Run each profiling contributing file once
	for v in $(PROFILE_BINARIES)
	do
		echo "v is set to: $${v}"
#		Get filename without path
		NAME=$$( echo $$v | xargs -n1 basename)
#		Get normal compilation of file without -pg flag
		NON_PG=$$( echo "$${v}" | sed 's/_profiling//g' )
		echo "NOT PROFILING binary: $${NON_PG}"
		mkdir -p $(PROFDIR)/$${GIT_COMMIT_ID}/$${NAME}/
		mkdir -p $(PROFDIR)/lists/M$(M)/N$(N)/$${NAME}/
#		Set how many times to run a file to reduce jitter in results
		count=20
#		For each time create a file with format profiling/<executable-file-name>_result_index
#		where index is loop step that is left zero padded to always be 3 digits long
		echo "Collecting Runtime profile for $$v"
		for i in $$(seq $$count)
		do
			echo "Executing: $${v}, Remaining executions: $${i}/$${count}..."
			INDEX=$$(printf %03d $${i})
#			run time command with verbose flag to get all info and save in file mentioned above
			RESULT=$$( ( ( command time -v $$v $(M) $(N); ) 1>/dev/null; ) 2>$(PROFDIR)/lists/M$(M)/N$(N)/$${NAME}/$${NAME}_result_$${INDEX}; )

#			if it is the first loop then move gmon.out to gmon.sum
			if [ $$i -eq 1 ]
			then
				mv gmon.out gmon.sum
			else
#				Otherwise merge new gmon.out into gmon.sum
				gprof -s $$v gmon.out gmon.sum
			fi
		done
		echo ""

#		Save all temporary result files into a variable
		TMP_RESULT_FILES=$$(find ./ -name "$${NAME}_result*")
#		for each file check the second line
#		if User time (seconds): contains a lower number than we have stored
#		update our runtime var and set file by dividing global line number by 23 due to each file being 23 lines long and casting to int
#		when we reach the end we print the id of the file with best User time (seconds): result.
		BEST_TIME=$$(echo $${TMP_RESULT_FILES} | xargs awk 'BEGIN{runtime = -1; file = 1;}{
			if (FNR == 2) {
				if (runtime == -1) {
					runtime = $$4+0;
				} else if (runtime > $$4+0) {
					runtime = $$4+0;
					file = int(NR / 23) + 1;
				}
			}
		} END {print sprintf("%03d", file);}')

#		rename the temporary file with best result into <Commit-sha>_<executable-name>_runtime
		mv $(PROFDIR)/lists/M$(M)/N$(N)/$${NAME}/$${NAME}_result_$${BEST_TIME} $(PROFDIR)/lists/M$(M)/N$(N)/$${NAME}/$${NAME}_runtime
#		Remove all other temporary files as they will not be needed
		rm -rf $${TMP_RESULT_FILES}
#		create report using the gmon.sum and save into <Commit-sha>_<executable-name>_function_usage_analysis
		gprof -b $$v gmon.sum > $(PROFDIR)/lists/M$(M)/N$(N)/$${NAME}/$${NAME}_function_usage_analysis


#		Get Heap profile		
		echo ""
		echo "Getting heap profile from $${NON_PG}"
		$(HEAP_RUNNER) $(HEAP_PRE_FLAGS) $${NON_PG} $(M) $(N) $(HEAP_POST_FLAGS)
		HEAP_OUTPUT=$$(find . -name "massif.out.*" | tail -1 | grep -o -e "[[:digit:]]\{1,\}")
		mv massif.out.$${HEAP_OUTPUT} $(PROFDIR)/$${GIT_COMMIT_ID}/data/$${NAME}_massif.data
		ms_print --threshold=0.01 "$(PROFDIR)/$${GIT_COMMIT_ID}/data/$${NAME}_massif.data"  > $(PROFDIR)/lists/M$(M)/N$(N)/$${NAME}/$${NAME}_heap_profile
		echo "Heap profile extracted from $${NON_PG}"
		echo ""
		

#		Get Cache profile
		echo ""
		echo "Getting cache profile from $${NON_PG}"
		$(CACHE_RUNNER) $(CACHE_PRE_FLAGS) $${NON_PG} $(M) $(N) > $(PROFDIR)/lists/M$(M)/N$(N)/$${NAME}/$${NAME}_cachegrind_output.txt 2>&1
		CACHE_OUTPUT=$$(find . -name "cachegrind.out.*" | tail -1 | grep -o -e "[[:digit:]]\{1,\}")
		mv cachegrind.out.$${CACHE_OUTPUT} $(PROFDIR)/$${GIT_COMMIT_ID}/data/$${NAME}_cachegrind.data
		cg_annotate "$(PROFDIR)/$${GIT_COMMIT_ID}/data/$${NAME}_cachegrind.data" > $(PROFDIR)/lists/M$(M)/N$(N)/$${NAME}/$${NAME}_cache_profile
		echo "Cache profile extracted from $${NON_PG}"
		echo ""; \
    done

	@echo "###"
	@echo "# Code Profiling Completed"
	@echo "# "
	@echo "# Results can be found in $(PROFDIR)/$${GIT_COMMIT_ID}/"
	@echo "###"

# Build all dependencies

###
# Executable rule's
###
$(BINDIR)/%_test: $(OBJDIR)/%_test.coverage.o $(OBJS_COV) $(OBJDIR)/oom.coverage.o $(OBJDIR)/ui_mocking.coverage.o | $(BINDIR)/
	@echo "Linking: $@"
	$(CC) -o $@ $(CFLAGS) $(CCOVFLAGS) $(IO_WRAP) $(MEMORY_WRAP) $^ $(LDFLAGS) $(CUNIT_LINK)

$(BINDIR)/%_profiling: $(OBJDIR)/%.profiling.o $(OBJS_PROF) | $(BINDIR)/
	@echo "Linking: $@"
	$(CC) -o $@ $(CFLAGS) $(CPROFFLAGS) $^ $(LDFLAGS)

$(BINDIR)/%: $(OBJDIR)/%.o $(OBJS) | $(BINDIR)/ $(OBJDIR)/
	@echo "Linking: $@"
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

###
# Profiling object files
###

define compile_with_profile_flags
	@echo "Compiling: $@"
	@$(CC) -o $@ -c $(CFLAGS) $(CPROFFLAGS) $<
	@${CC} -MM -MP $< -MT "$@" -o $(DEPDIR)/${shell basename ${basename $@}}.d
	@sed "s/src\/..\///g" $(DEPDIR)/${shell basename ${basename $@}}.d  > $(DEPDIR)/${shell basename ${basename $@}}.tmp.d
	@sed "s/demo\/..\///g" $(DEPDIR)/${shell basename ${basename $@}}.tmp.d > $(DEPDIR)/${shell basename ${basename $@}}.tmp2.d
	@sed "s/test\/..\///g" $(DEPDIR)/${shell basename ${basename $@}}.tmp2.d  > $(DEPDIR)/${shell basename ${basename $@}}.new.d
	@rm $(DEPDIR)/${shell basename ${basename $@}}.tmp2.d
	@rm $(DEPDIR)/${shell basename ${basename $@}}.tmp.d
	@mv $(DEPDIR)/${shell basename ${basename $@}}.new.d $(DEPDIR)/${shell basename ${basename $@}}.d
	@touch $@
endef

$(OBJDIR)/%.profiling.o: $(SRCDIR)/%.c | $(OBJDIR)/ $(DEPDIR)/
	$(compile_with_profile_flags)

$(OBJDIR)/%.profiling.o: $(TESTDIR)/%.c | $(OBJDIR)/ $(DEPDIR)/
	$(compile_with_profile_flags)

$(OBJDIR)/%.profiling.o: $(LIBDIR)/%.c | $(OBJDIR)/ $(DEPDIR)/
	$(compile_with_profile_flags)

$(OBJDIR)/%.profiling.o: $(DEMODIR)/$(SRCDIR)/%.c | $(OBJDIR)/ $(DEPDIR)/
	$(compile_with_profile_flags)

$(OBJDIR)/%.profiling.o: $(DEMODIR)/$(TESTDIR)/%.c | $(OBJDIR)/ $(DEPDIR)/
	$(compile_with_profile_flags)


###
# Coverage object files
###

define compile_with_coverage_flags
	@echo "Compiling: $@"
	@$(CC) -o $@ -c $(CFLAGS) $(CCOVFLAGS) $<
	@${CC} -MM -MP $< -MT "$@" -o $(DEPDIR)/${shell basename ${basename $@}}.d
	@sed "s/src\/..\///g" $(DEPDIR)/${shell basename ${basename $@}}.d  > $(DEPDIR)/${shell basename ${basename $@}}.tmp.d
	@sed "s/demo\/..\///g" $(DEPDIR)/${shell basename ${basename $@}}.tmp.d > $(DEPDIR)/${shell basename ${basename $@}}.tmp2.d
	@sed "s/test\/..\///g" $(DEPDIR)/${shell basename ${basename $@}}.tmp2.d  > $(DEPDIR)/${shell basename ${basename $@}}.new.d
	@rm $(DEPDIR)/${shell basename ${basename $@}}.tmp2.d
	@rm $(DEPDIR)/${shell basename ${basename $@}}.tmp.d
	@mv $(DEPDIR)/${shell basename ${basename $@}}.new.d $(DEPDIR)/${shell basename ${basename $@}}.d
	@touch $@
endef

$(OBJDIR)/%.coverage.o: $(SRCDIR)/%.c | $(OBJDIR)/ $(DEPDIR)/
	$(compile_with_coverage_flags)

$(OBJDIR)/%.coverage.o: $(TESTDIR)/%.c | $(OBJDIR)/ $(DEPDIR)/
	$(compile_with_coverage_flags)

$(OBJDIR)/%.coverage.o: $(LIBDIR)/%.c | $(OBJDIR)/ $(DEPDIR)/
	$(compile_with_coverage_flags)

$(OBJDIR)/%.coverage.o: $(DEMODIR)/$(SRCDIR)/%.c | $(OBJDIR)/ $(DEPDIR)/
	$(compile_with_coverage_flags)

$(OBJDIR)/%.coverage.o: $(DEMODIR)/$(TESTDIR)/%.c | $(OBJDIR)/ $(DEPDIR)/
	$(compile_with_coverage_flags)


###
# Production object files
###

define compile_with_normal_flags
	@echo "Compiling: $@"
	@$(CC) -o $@ -c $(CFLAGS) $<
	@${CC} -MM -MP $< -MT "$@" -o $(DEPDIR)/${shell basename ${basename $@}}.d
	@sed "s/src\/..\///g" $(DEPDIR)/${shell basename ${basename $@}}.d  > $(DEPDIR)/${shell basename ${basename $@}}.tmp.d
	@sed "s/demo\/..\///g" $(DEPDIR)/${shell basename ${basename $@}}.tmp.d > $(DEPDIR)/${shell basename ${basename $@}}.tmp2.d
	@sed "s/test\/..\///g" $(DEPDIR)/${shell basename ${basename $@}}.tmp2.d  > $(DEPDIR)/${shell basename ${basename $@}}.new.d
	@rm $(DEPDIR)/${shell basename ${basename $@}}.tmp2.d
	@rm $(DEPDIR)/${shell basename ${basename $@}}.tmp.d
	@mv $(DEPDIR)/${shell basename ${basename $@}}.new.d $(DEPDIR)/${shell basename ${basename $@}}.d
	@touch $@
endef

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)/ $(DEPDIR)/
	$(compile_with_normal_flags)

$(OBJDIR)/%.o: $(TESTDIR)/%.c | $(OBJDIR)/ $(DEPDIR)/
	$(compile_with_normal_flags)

$(OBJDIR)/%.o: $(LIBDIR)/%.c | $(OBJDIR)/ $(DEPDIR)/
	$(compile_with_normal_flags)

$(OBJDIR)/%.o: $(DEMODIR)/$(SRCDIR)/%.c | $(OBJDIR)/ $(DEPDIR)/
	$(compile_with_normal_flags)

$(OBJDIR)/%.o: $(DEMODIR)/$(TESTDIR)/%.c | $(OBJDIR)/ $(DEPDIR)/
	$(compile_with_normal_flags)


# Includes rules created from .c files by reading include headers
-include ${DEPS}

###
# Directory Rules
###

%/:
	mkdir -p $@