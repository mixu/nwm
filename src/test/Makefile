# to run:
# make clean all test


#Set this to @ to keep the makefile quiet
SILENCE = @

#---- Outputs ----#
COMPONENT_NAME = t1
TARGET_LIB = \
	lib/lib$(COMPONENT_NAME).a

TEST_TARGET = \
	$(COMPONENT_NAME)_tests

#--- Inputs ----#
PROJECT_HOME_DIR = .
CPPUTEST_HOME = /home/m/other/CppUTest
CPP_PLATFORM = Gcc

CPPUTEST_USE_EXTENSIONS = Y
CPPUTEST_WARNINGFLAGS += -Wall
CPPUTEST_WARNINGFLAGS += -Werror
CPPUTEST_WARNINGFLAGS += -Wswitch-default
CPPUTEST_WARNINGFLAGS += -Wswitch-enum
CPPUTEST_CFLAGS += -std=c89
CPPUTEST_CFLAGS += -Wextra
CPPUTEST_CFLAGS += -pedantic
CPPUTEST_CFLAGS += -Wstrict-prototypes

SRC_DIRS = \
	src/*\
	src/example\

TEST_SRC_DIRS = \
	tests \
	tests/test\

INCLUDE_DIRS =\
  .\
  include/*\
  $(CPPUTEST_HOME)/include/

include $(CPPUTEST_HOME)/build/MakefileWorker.mk
