#!/bin/bash

# Combined diagnostic and cleanup script for quick troubleshooting

echo "=========================================="
echo "Stress Test Diagnostic & Cleanup"
echo "=========================================="
echo ""

cd "$(dirname "$0")"

# Step 1: Check for existing model files
echo "Step 1: Checking for existing model files..."
MODEL_COUNT=$(ls -1 *.bin 2>/dev/null | wc -l)
if [ $MODEL_COUNT -gt 0 ]; then
    echo "⚠ Found $MODEL_COUNT existing model file(s):"
    ls -lh *.bin 2>/dev/null | awk '{print "  " $9 " (" $5 ")"}'
    TOTAL_SIZE=$(du -sh *.bin 2>/dev/null | tail -1 | awk '{print $1}')
    echo "  Total size: $TOTAL_SIZE"
    echo ""
    echo "These files may interfere with tests."
    read -p "Delete them now? (y/N) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        rm -f *.bin
        echo "✓ Model files deleted"
    else
        echo "⚠ Keeping existing files (may cause issues)"
    fi
else
    echo "✓ No existing model files found"
fi

echo ""
echo "=========================================="
echo ""

# Step 2: Run diagnostics
echo "Step 2: Running diagnostics..."
echo ""

if [ -f "./diagnose_test4.sh" ]; then
    bash ./diagnose_test4.sh
    DIAG_EXIT=$?
    
    echo ""
    echo "=========================================="
    echo ""
    
    if [ $DIAG_EXIT -eq 0 ]; then
        echo "✓ All diagnostic tests passed!"
        echo ""
        echo "Your system is ready to run stress tests."
        echo ""
        echo "Next steps:"
        echo "  1. Run individual tests: ./test_large_scale.sh"
        echo "  2. Run all tests: ./run_stress_tests.sh"
        echo "  3. See QUICK_START.md for more options"
    else
        echo "✗ Some diagnostic tests failed"
        echo ""
        echo "Please review the error output above."
        echo ""
        echo "Common solutions:"
        echo "  - Missing binaries: Run 'make all' in root directory"
        echo "  - Missing data: Check tests/data/ directory"
        echo "  - Disk space: Free up space or run ./cleanup.sh"
        echo "  - Memory: Close other applications"
        echo ""
        echo "See FIXES_APPLIED.md for detailed troubleshooting"
    fi
else
    echo "✗ diagnose_test4.sh not found"
    echo "   Please ensure you're in the tests/stress directory"
    exit 1
fi

echo ""
echo "=========================================="
