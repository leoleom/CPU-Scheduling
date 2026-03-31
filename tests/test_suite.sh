#!/bin/bash
# CPU Scheduling Simulator Test Suite (Final Integrated Version)

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

TOTAL_TESTS=0
PASSED_TESTS=0

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
EXECUTABLE="$PROJECT_ROOT/schedsim"

if [ ! -f "$EXECUTABLE" ]; then
    echo -e "${RED}Error: schedsim not found. Run make first.${NC}"
    exit 1
fi

cd "$PROJECT_ROOT"

pass() {
    echo -e "${GREEN}PASS${NC}"
    PASSED_TESTS=$((PASSED_TESTS + 1))
}

fail() {
    echo -e "${RED}FAIL${NC} - $1"
}

test_header() {
    echo ""
    echo -e "${YELLOW}$1${NC}"
    echo "=================================="
}

check_avg() {
    local name="$1"
    local command="$2"
    local expected="$3"

    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    echo -n "$name (expected $expected): "

    $command > /tmp/out.log 2>&1

    if grep -E -q "$expected" /tmp/out.log; then
        pass
    else
        fail "Expected avg TT = $expected"
        tail -5 /tmp/out.log | sed 's/^/    /'
    fi
}

########################################
echo -e "${BLUE}CPU Scheduling Test Suite${NC}"
echo "=================================="


# Quiz 4
test_header "Quiz 4 Workload"
check_avg "FCFS" "$EXECUTABLE --algorithm=FCFS --input=tests/quiz4.txt" "515"
check_avg "SJF"  "$EXECUTABLE --algorithm=SJF --input=tests/quiz4.txt" "461"
check_avg "STCF" "$EXECUTABLE --algorithm=STCF --input=tests/quiz4.txt" "393"
check_avg "RR q=30" "$EXECUTABLE --algorithm=RR --input=tests/quiz4.txt --quantum=30" "627"

# Single
test_header "Single Process"
check_avg "FCFS" "$EXECUTABLE --algorithm=FCFS --input=tests/single_process.txt" "100"
check_avg "SJF"  "$EXECUTABLE --algorithm=SJF --input=tests/single_process.txt" "100"
check_avg "STCF" "$EXECUTABLE --algorithm=STCF --input=tests/single_process.txt" "100"
check_avg "RR q=30" "$EXECUTABLE --algorithm=RR --input=tests/single_process.txt --quantum=30" "100"

# Staircase
test_header "Staircase Arrivals"
check_avg "FCFS" "$EXECUTABLE --algorithm=FCFS --input=tests/staircase_arrivals.txt" "58"
check_avg "SJF"  "$EXECUTABLE --algorithm=SJF --input=tests/staircase_arrivals.txt" "58"
check_avg "STCF" "$EXECUTABLE --algorithm=STCF --input=tests/staircase_arrivals.txt" "58"
check_avg "RR q=30" "$EXECUTABLE --algorithm=RR --input=tests/staircase_arrivals.txt --quantum=30" "58"

# Simultaneous
test_header "Simultaneous Arrivals"
check_avg "FCFS" "$EXECUTABLE --algorithm=FCFS --input=tests/simultaneous_arrivals.txt" "24.3"
check_avg "SJF"  "$EXECUTABLE --algorithm=SJF --input=tests/simultaneous_arrivals.txt" "16.3"
check_avg "STCF" "$EXECUTABLE --algorithm=STCF --input=tests/simultaneous_arrivals.txt" "16.3"
check_avg "RR q=30" "$EXECUTABLE --algorithm=RR --input=tests/simultaneous_arrivals.txt --quantum=30" "24.3"

# Identical
test_header "Identical Burst"
check_avg "FCFS" "$EXECUTABLE --algorithm=FCFS --input=tests/identical_burst.txt" "97.7"
check_avg "SJF"  "$EXECUTABLE --algorithm=SJF --input=tests/identical_burst.txt" "97.7"
check_avg "STCF" "$EXECUTABLE --algorithm=STCF --input=tests/identical_burst.txt" "97.7"
check_avg "RR q=30" "$EXECUTABLE --algorithm=RR --input=tests/identical_burst.txt --quantum=30" "127.7"

# Zero_Burst
test_header "Zero Burst Time"
check_avg "FCFS" "$EXECUTABLE --algorithm=FCFS --input=tests/zero_burst.txt" "0"
check_avg "SJF"  "$EXECUTABLE --algorithm=SJF --input=tests/zero_burst.txt" "0"
check_avg "STCF" "$EXECUTABLE --algorithm=STCF --input=tests/zero_burst.txt" "0"
check_avg "RR q=30" "$EXECUTABLE --algorithm=RR --input=tests/zero_burst.txt --quantum=30" "0"  

# Large
test_header "Large Workload"
check_avg "FCFS" "$EXECUTABLE --algorithm=FCFS --input=tests/large_workload.txt" "77402.7"
check_avg "SJF"  "$EXECUTABLE --algorithm=SJF --input=tests/large_workload.txt" "4865.8"
check_avg "STCF" "$EXECUTABLE --algorithm=STCF --input=tests/large_workload.txt" "4814.6"
check_avg "RR q=30" "$EXECUTABLE --algorithm=RR --input=tests/large_workload.txt --quantum=30" "9529.5"

########################################
# Summary
########################################

echo ""
echo -e "${BLUE}==================================${NC}"
echo -e "${BLUE}TEST SUMMARY${NC}"
echo -e "${BLUE}==================================${NC}"

echo "Total: $TOTAL_TESTS"
echo "Passed: $PASSED_TESTS"
echo "Failed: $((TOTAL_TESTS - PASSED_TESTS))"

if [ $TOTAL_TESTS -eq $PASSED_TESTS ]; then
    echo -e "${GREEN}All tests passed!${NC}"
else
    echo -e "${RED}Some tests failed.${NC}"
fi