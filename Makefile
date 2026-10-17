CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic -Isrc -Ideps
SRCS = src/Expense.cpp src/ExpenseNode.cpp src/ExpenseManager.cpp 

test:
	$(CXX) $(CXXFLAGS) $(SRCS) tests/person1_test.cpp -o tests/run_test
	./tests/run_test
	rm -f tests/run_test

clean:
	rm -f tests/run_test
