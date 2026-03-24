set -e

BINARY="../build/schedsim"
TESTS_DIR="."
OUTPUT_FILE="workload_results.txt"
ANALYSIS_FILE="workload_analysis.txt"

GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

if [ ! -f "$BINARY" ]; then
    echo -e "${RED}Error: Binary not found at $BINARY${NC}"
    echo "Please build the project first with: make"
    exit 1
fi

echo -e "${BLUE}=== MLFQ Workload Pattern Analysis ===${NC}\n"

declare -A workloads=(
    ["short"]="workload_short.txt|All short jobs (burst < 50)"
    ["long"]="workload_long.txt|All long jobs (burst > 200)"
    ["mixed"]="workload_mixed.txt|Mixed workload (50% short, 50% long)"
    ["bimodal"]="workload_bimodal.txt|Bimodal distribution (many short + few long)"
    ["io_bound"]="workload_io_bound.txt|I/O-bound pattern (regular CPU bursts)"
)

> "$OUTPUT_FILE"
> "$ANALYSIS_FILE"

run_workload_test() {
    local workload_name="$1"
    local workload_file="$2"
    local description="$3"
    local algorithm="$4"
    
    if [ ! -f "$TESTS_DIR/$workload_file" ]; then
        echo -e "${RED}✗ Workload file not found: $workload_file${NC}"
        return 1
    fi
    
    echo -e "${BLUE}Testing: $description - Algorithm: $algorithm${NC}"
    echo "Workload: $workload_name | Algorithm: $algorithm" >> "$OUTPUT_FILE"
    echo "Description: $description" >> "$OUTPUT_FILE"
    
    $BINARY --algorithm="$algorithm" --input="$TESTS_DIR/$workload_file" >> "$OUTPUT_FILE" 2>&1
    echo "---" >> "$OUTPUT_FILE"
}

for workload_key in "${!workloads[@]}"; do
    IFS='|' read -r workload_file description <<< "${workloads[$workload_key]}"
    
    echo -e "\n${YELLOW}========================================${NC}"
    echo -e "${YELLOW}Workload: $workload_key - $description${NC}"
    echo -e "${YELLOW}========================================${NC}\n"
    
    echo "WORKLOAD PATTERN: $description" >> "$ANALYSIS_FILE"
    echo "Workload File: $workload_file" >> "$ANALYSIS_FILE"
    echo "" >> "$ANALYSIS_FILE"
    
    for algo in FCFS SJF STCF RR MLFQ; do
        run_workload_test "$workload_key" "$workload_file" "$description" "$algo"
    done
    
    echo "" >> "$ANALYSIS_FILE"
done

echo ""
echo -e "${GREEN}✓ All workload tests completed${NC}"
echo -e "${BLUE}Raw results saved to: $OUTPUT_FILE${NC}"
echo -e "${BLUE}Analysis saved to: $ANALYSIS_FILE${NC}"
echo ""
echo "To view results:"
echo "  Raw data:  cat $OUTPUT_FILE"
echo "  Analysis:  cat $ANALYSIS_FILE"
