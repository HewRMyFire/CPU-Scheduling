set -e

BINARY="../build/schedsim"
TESTS_DIR="."

GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m'

if [ ! -f "$BINARY" ]; then
    echo -e "${RED}Error: Binary not found at $BINARY${NC}"
    echo "Please build the project first with: make"
    exit 1
fi

echo -e "${BLUE}=== CPU Scheduling Algorithm Test Suite ===${NC}\n"

TESTS_RUN=0
TESTS_PASSED=0
TESTS_FAILED=0

run_test() {
    local algo="$1"
    local workload="$2"
    local description="$3"
    
    TESTS_RUN=$((TESTS_RUN + 1))
    
    if [ ! -f "$TESTS_DIR/$workload" ]; then
        echo -e "${RED}✗ Test ${TESTS_RUN}: $description${NC}"
        echo "  Workload file not found: $TESTS_DIR/$workload"
        TESTS_FAILED=$((TESTS_FAILED + 1))
        return
    fi
    
    echo -e "${BLUE}Running Test ${TESTS_RUN}: $description${NC}"
    echo "  Algorithm: $algo"
    echo "  Workload:  $workload"
    
    if $BINARY --algorithm="$algo" --input="$TESTS_DIR/$workload"; then
        echo -e "${GREEN}✓ PASSED${NC}\n"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗ FAILED${NC}\n"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
}

run_test "FCFS" "workload5.txt" "First Come First Served (Convoy Effect)"
run_test "SJF" "workload3.txt" "Shortest Job First (Optimal Scheduling)"
run_test "RR" "workload4.txt" "Round Robin (Fair Time Slices)"
run_test "STCF" "workload2.txt" "Shortest Time-to-Completion First (Preemption)"
run_test "MLFQ" "workload1.txt" "Multi-Level Feedback Queue"

echo -e "${BLUE}Running Test 6: Comparison Mode (workload1)${NC}"
TESTS_RUN=$((TESTS_RUN + 1))
if $BINARY --compare --input="$TESTS_DIR/workload1.txt"; then
    echo -e "${GREEN}✓ PASSED${NC}\n"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "${RED}✗ FAILED${NC}\n"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

echo -e "${BLUE}Running Test 7: Comparison Mode (workload2)${NC}"
TESTS_RUN=$((TESTS_RUN + 1))
if $BINARY --compare --input="$TESTS_DIR/workload2.txt"; then
    echo -e "${GREEN}✓ PASSED${NC}\n"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "${RED}✗ FAILED${NC}\n"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

echo -e "${BLUE}Running Test 8: Comparison Mode (workload3)${NC}"
TESTS_RUN=$((TESTS_RUN + 1))
if $BINARY --compare --input="$TESTS_DIR/workload3.txt"; then
    echo -e "${GREEN}✓ PASSED${NC}\n"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "${RED}✗ FAILED${NC}\n"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

echo -e "${BLUE}Running Test 9: Comparison Mode (workload4)${NC}"
TESTS_RUN=$((TESTS_RUN + 1))
if $BINARY --compare --input="$TESTS_DIR/workload4.txt"; then
    echo -e "${GREEN}✓ PASSED${NC}\n"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "${RED}✗ FAILED${NC}\n"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

echo -e "${BLUE}Running Test 10: Comparison Mode (workload5)${NC}"
TESTS_RUN=$((TESTS_RUN + 1))
if $BINARY --compare --input="$TESTS_DIR/workload5.txt"; then
    echo -e "${GREEN}✓ PASSED${NC}\n"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "${RED}✗ FAILED${NC}\n"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

echo -e "${BLUE}=== Test Summary ===${NC}"
echo "Total Tests: $TESTS_RUN"
echo -e "Passed: ${GREEN}$TESTS_PASSED${NC}"
echo -e "Failed: ${RED}$TESTS_FAILED${NC}"

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "\n${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "\n${RED}Some tests failed!${NC}"
    exit 1
fi
