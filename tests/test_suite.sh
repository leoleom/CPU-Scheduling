#!/bin/bash
# test_suite.sh

echo "Running CMSC 125 Lab 2 Test Suite..."

# Test 1: Verify against lecture examples
echo "Test 1: Lecture Quiz 4 Workload"
./schedsim --algorithm=FCFS --input=tests/quiz4.txt > /tmp/fcfs.txt
if grep -q "Average.*750" /tmp/fcfs.txt; then
    echo "  FCFS: PASS"
else
    echo "  FCFS: FAIL (expected avg TT = 750)"
fi

# Test 2: Edge cases
echo "Test 2: Edge Cases"
./schedsim --algorithm=STCF --processes="A:0:0" > /dev/null
if [ $? -eq 0 ]; then
    echo "  Zero burst time: PASS"
else
    echo "  Zero burst time: FAIL"
fi

echo "Test suite complete."
echo "Note: Memory leaks will be checked via code review during defense."
