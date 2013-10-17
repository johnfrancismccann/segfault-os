ALL: cat grep hello ls pingpong counter shell sigtest testprint syserr

%.o: %.c
	gcc -c -Wall -o $@ $<

%.o: %.S
	gcc -c -Wall -o $@ $<

cat.exe: ece391cat.o ece391syscall.o  ece391support.o
	gcc -nostdlib -o cat.exe ece391cat.o ece391syscall.o ece391support.o
cat: cat.exe
	../elfconvert cat.exe
	mv cat.exe.converted to_fsdir/cat

grep.exe: ece391grep.o ece391syscall.o  ece391support.o
	gcc -nostdlib -o grep.exe ece391grep.o ece391syscall.o ece391support.o
grep: grep.exe
	../elfconvert grep.exe
	mv grep.exe.converted to_fsdir/grep

hello.exe: ece391hello.o ece391syscall.o  ece391support.o
	gcc -nostdlib -o hello.exe ece391hello.o ece391syscall.o ece391support.o
hello: hello.exe
	../elfconvert hello.exe
	mv hello.exe.converted to_fsdir/hello

ls.exe: ece391ls.o ece391syscall.o  ece391support.o
	gcc -nostdlib -o ls.exe ece391ls.o ece391syscall.o ece391support.o
ls: ls.exe
	../elfconvert ls.exe
	mv ls.exe.converted to_fsdir/ls

pingpong.exe: ece391pingpong.o ece391syscall.o  ece391support.o
	gcc -nostdlib -o pingpong.exe ece391pingpong.o ece391syscall.o ece391support.o
pingpong: pingpong.exe
	../elfconvert pingpong.exe
	mv pingpong.exe.converted to_fsdir/pingpong

counter.exe: ece391counter.o ece391syscall.o  ece391support.o
	gcc -nostdlib -o counter.exe ece391counter.o ece391syscall.o ece391support.o
counter: counter.exe
	../elfconvert counter.exe
	mv counter.exe.converted to_fsdir/counter

shell.exe: ece391shell.o ece391syscall.o  ece391support.o
	gcc -nostdlib -o shell.exe ece391shell.o ece391syscall.o ece391support.o
shell: shell.exe
	../elfconvert shell.exe
	mv shell.exe.converted to_fsdir/shell

sigtest.exe: ece391sigtest.o ece391syscall.o  ece391support.o
	gcc -nostdlib -o sigtest.exe ece391sigtest.o ece391syscall.o ece391support.o
sigtest: sigtest.exe
	../elfconvert sigtest.exe
	mv sigtest.exe.converted to_fsdir/sigtest
	
testprint.exe: ece391testprint.o ece391syscall.o  ece391support.o
	gcc -nostdlib -o testprint.exe ece391testprint.o ece391syscall.o ece391support.o
testprint: testprint.exe
	../elfconvert testprint.exe
	mv testprint.exe.converted to_fsdir/testprint
	
syserr.exe: ece391syserr.o ece391syscall.o  ece391support.o
	gcc -nostdlib -o syserr.exe ece391syserr.o ece391syscall.o ece391support.o
syserr: syserr.exe
	../elfconvert syserr.exe
	mv syserr.exe.converted to_fsdir/syserr

clean::
	rm -f *~ *.o

clean::
	rm -f *~ *.o

clear: clean
	rm -f *.converted
	rm -f *.exe
	rm -f to_fsdir/*

