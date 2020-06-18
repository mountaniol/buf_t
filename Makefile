GCC=gcc
AR=ar
#GCC=clang-10
CFLAGS=-Wall -Wextra -rdynamic -O2
DEBUG=-DDEBUG3
DEBUG += -DDERROR3
#CFLAGS += -fanalyzer

#GCCVERSION=$(shell gcc -dumpversion | sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g' -e 's/^[0-9]\{3,4\}$/&00/')

DEBUG=-DDEBUG3

# client daemon

BUF_O=buf_t.o buf_t_memory.o
BUF_A=buf_t.a
BUF_TEST=btest.out

# Files to check with splint
SPLINT_C=buf_t.c buf_t.h buf_t_memory.c buf_t_memory.h buf_t_debug.h

all: buf
	@echo "Version = $(GCCVERSION)"
	@echo "DEBUG = $(DEBUG)"
	@echo "CFLAGS = $(CFLAGS)"
	@echo "Compiler = $(GCC)"

buf: $(BUF_O)
	@echo Done
	$(AR) rcs $(BUF_A) $(BUF_O)

btest: $(BUF_O)
	$(GCC) $(CFLAGS) -ggdb -DSTANDALONE $(DEBUG) $(BUF_O) buf_t_test.c  -o $(BUF_TEST)

btest_a: buf
	$(GCC) $(CFLAGS) -ggdb $(DEBUG) buf_t_test.c $(BUF_A) -o $(BUF_TEST)

clean:
	rm -f *.o $(BUF_A) $(BUF_TEST) 


.PHONY:check
check:
	@echo "+++ $@: USER=$(USER), UID=$(UID), GID=$(GID): $(CURDIR)"
	#echo ============= 32 bit check =============
	$(ECH)cppcheck -j2 -q --force  --enable=all --platform=unix32 -I/usr/include/openssl $(SPLINT_C)       
	#echo ============= 64 bit check =============
	#$(ECH)cppcheck -q --force  --enable=all --platform=unix64 -I/usr/include/openssl ./*.[ch]

.PHONY:splint
splint:
	@echo "+++ $@: USER=$(USER), UID=$(UID), GID=$(GID): $(CURDIR)"
	splint -noeffect -mustfreeonly -forcehints -weak -redef +matchanyintegral +gnuextensions -preproc +unixlib -I/usr/include/openssl -D__gnuc_va_list=va_list  ./*.[ch]
	#splint -standard -noeffect -redef +matchanyintegral +gnuextensions -preproc +unixlib  +trytorecover -mayaliasunique +posixlib -I/usr/include/openssl -D__gnuc_va_list=va_list $(SPLINT_C)
	#splint -forcehints -standard -redef -exportlocal -export-header -isoreserved  -preproc +unixlib -I/usr/include/openssl -D__gnuc_va_list=va_list  ./*.[ch]
	#splint -checks -redef -exportlocal -export-header -isoreserved  -preproc +unixlib -I/usr/include/openssl -D__gnuc_va_list=va_list  ./*.[ch]
	#splint -forcehints -weak -redef +matchanyintegral +gnuextensions -preproc +unixlib -I/usr/include/openssl -D__gnuc_va_list=va_list  ./*.[ch]
flaw:
	flawfinder ./*.[ch] 

%.o:%.c
	@echo "|>" $@...
	@$(GCC) -g $(INCLUDE) $(CFLAGS) $(DEBUG) -c -o $@ $<


