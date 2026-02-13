#!/bin/bash

# Cleanup script for stress test artifacts

echo "=== Stress Test Cleanup ==="
echo ""

cd "$(dirname "$0")"

# Count and show model files
MODEL_COUNT=$(ls -1 *.bin 2>/dev/null | wc -l)
if [ $MODEL_COUNT -gt 0 ]; then
    echo "Found $MODEL_COUNT model file(s):"
    ls -lh *.bin 2>/dev/null | awk '{print "  " $9 " (" $5 ")"}'
    
    TOTAL_SIZE=$(du -sh *.bin 2>/dev/null | awk '{sum+=$1} END {print sum}')
    echo ""
    echo "Total size: $(du -sh *.bin 2>/dev/null | tail -1 | awk '{print $1}')"
    echo ""
    
    read -p "Delete all model files? (y/N) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        rm -f *.bin
        echo "✓ Model files deleted"
    else
        echo "Cleanup cancelled"
        exit 0
    fi
else
    echo "No model files found"
fi

# Clean up temporary files
TEMP_COUNT=$(ls -1 /tmp/*_output.txt 2>/dev/null | wc -l)
if [ $TEMP_COUNT -gt 0 ]; then
    echo ""
    echo "Found $TEMP_COUNT temporary output file(s)"
    rm -f /tmp/*_output.txt
    echo "✓ Temporary files deleted"
fi

# Clean up test result files
if [ -f "edge_results.txt" ]; then
    rm -f edge_results.txt
    echo "✓ Test result files deleted"
fi

if [ -f "batch_results.txt" ]; then
    rm -f batch_results.txt
fi

echo ""
echo "=== Cleanup Complete ==="
