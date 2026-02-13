#include <gtest/gtest.h>
#include "embedding/embedding_table.h"

TEST(EmbeddingTableTest, Construction) {
    EmbeddingTable table(1000, 50, 42);
    
    EXPECT_EQ(table.bucket_count(), 1000);
    EXPECT_EQ(table.dim(), 50);
    EXPECT_EQ(table.memory_bytes(), 1000 * 50 * sizeof(float));
}

TEST(EmbeddingTableTest, RowAccess) {
    EmbeddingTable table(100, 10, 42);
    
    float* row0 = table.row(0);
    float* row1 = table.row(1);
    
    EXPECT_NE(row0, row1);
    EXPECT_EQ(row1 - row0, 10); // Adjacent rows
}

TEST(EmbeddingTableTest, Deterministic) {
    EmbeddingTable table1(100, 10, 42);
    EmbeddingTable table2(100, 10, 42);
    
    const float* row1 = table1.row(0);
    const float* row2 = table2.row(0);
    
    for (int i = 0; i < 10; ++i) {
        EXPECT_FLOAT_EQ(row1[i], row2[i]);
    }
}
