# This makefile uses some semi-advanced GNU make features to automatically generate the dependencies
# of the C source files it compiles. For a detailed guide, see the tutorial at:
#   http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/
#
# Common makefile macros:
#   $@ - the file to be made
#   $< - the related file that caused the action
#   $* - the prefix shared by the target and dependent files (e.g., 'foo' for 'foo.o: foo.c')

CC = gcc
LEX = flex
YACC = bison
DEBUG = -g
PROFILE = -pg
CFLAGS = -Wall -Wextra $(DEBUG) -std=gnu99 -Iinclude
LFLAGS = -Wall -Wextra $(DEBUG) -lm -lfl -lreadline
# These flags tell gcc to generate a dependency flag while compiling.
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td

DEPDIR = build/depend
ODIR = build
SDIR = src
IDIR = include
# Make sure that the build directories always exist.
$(shell mkdir -p $(ODIR)/scamval >/dev/null)
$(shell mkdir -p $(DEPDIR)/scamval >/dev/null)

POSTCOMPILE = @mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d && touch $@

EXECS = scam tests run_test_script benchmark compile
_OBJS = builtins.o collector.o eval.o grammar.o flex.o scamval/cmp.o scamval/dict.o scamval/misc.o \
	scamval/num.o scamval/seq.o scamval/str.o
# This just saves me the trouble of writing $(ODIR)/builtins.o, $(ODIR)/collector.o etc.
OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))


all: $(EXECS)

scam: $(ODIR)/scam.o $(OBJS)
	$(CC) $^ $(LFLAGS) -o $@

compile: $(ODIR)/compile.o $(OBJS)
	$(CC) $^ $(LFLAGS) -o $@

benchmark: $(ODIR)/benchmark.o $(OBJS)
	$(CC) $^ $(LFLAGS) -o $@

tests: $(ODIR)/tests.o $(OBJS)
	$(CC) $^ $(LFLAGS) -o $@

run_test_script: $(ODIR)/run_test_script.o $(OBJS)
	$(CC) $^ $(LFLAGS) -o $@

$(SDIR)/flex.c flex.h: $(SDIR)/grammar.l
	$(LEX) $<
	mv flex.c $(SDIR)
	mv flex.h $(IDIR)

$(SDIR)/grammar.c $(IDIR)/grammar.h: $(SDIR)/grammar.y
	$(YACC) -d $<
	mv grammar.c $(SDIR)
	mv grammar.h $(IDIR)

$(ODIR)/%.o: $(SDIR)/%.c $(DEPDIR)/%.d
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@
	$(POSTCOMPILE)

.PHONY: clean
clean:
	rm -f $(OBJS) $(SDIR)/flex.c $(SDIR)/grammar.c $(IDIR)/grammar.h $(EXECS)
	rm -rf $(DEPDIR)/*

# I do not know what these two lines do.
$(DEPDIR)/%.d: ;
.PRECIOUS: $(DEPDIR)/%.d

# This includes all the dependency makefiles that gcc automatically generated.
include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(_OBJS))))
