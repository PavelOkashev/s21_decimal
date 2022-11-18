CFLAGS = -Wall -Werror -Wextra -std=c11

all: test

s21_decimal: s21_decimal.c s21_decimal.h
	gcc -c $(CFLAGS) s21_decimal.c -o s21_decimal.o

s21_decimal.a: s21_decimal.o s21_decimal.h
	ar rcs s21_decimal.a s21_decimal.o
	ranlib s21_decimal.a

test: s21_decimal.a
	gcc s21_decimal.a test.c -lcheck $(CFLAGS) -o test.o
	./test.o

clean:
	rm -rf *.o s21_decimal.a *.out report gcov_report

gcov_report: gcov_report_build
	rm -rf *.gcda
	rm -rf *.gcno
	rm -rf *.info

gcov_report_build:
	gcc $(CFLAGS) --coverage test.c s21_decimal.c -o gcov_report -lcheck
	./gcov_report
	lcov -t "./gcov_report" -o gcov_report.info -c -d .
	genhtml -o report gcov_report.info
	open ./report/index.html

rebuild: clean all