#!/bin/bash

set -e

echo "=========================================="
echo "Transfer Learning Example"
echo "Pretrain + Fine-tune for Classification"
echo "=========================================="
echo

# Step 1: Create large unlabeled corpus
echo "Step 1: Creating large unlabeled corpus..."
cat > large_corpus.txt << 'EOF'
the quick brown fox jumps over the lazy dog
the dog runs fast in the park every morning
a cat sits on the mat and sleeps peacefully
the fox is quick and clever animal
dogs and cats are wonderful pets
the lazy cat sleeps all day long
quick brown animals run very fast
the park is beautiful in spring
morning walks are good for health
animals need love and care
wonderful pets bring joy to life
peaceful sleep is important for cats
clever foxes hunt at night
beautiful spring flowers bloom everywhere
health and happiness go together
joy and love make life better
important decisions need careful thought
better days are coming soon
fast runners win the race
good health requires exercise
exercise and diet are important
diet affects your health
spring brings new life
new opportunities arise daily
daily practice improves skills
skills development takes time
time management is crucial
crucial decisions shape future
future looks bright and promising
promising results encourage effort
effort leads to success
success brings happiness
happiness is the goal
goal setting helps planning
planning prevents problems
problems have solutions
solutions require creativity
creativity sparks innovation
innovation drives progress
progress benefits everyone
everyone deserves respect
respect builds relationships
relationships need trust
trust takes time
time heals wounds
wounds make us stronger
stronger together we stand
stand up for justice
justice for all people
people need compassion
compassion creates peace
peace brings harmony
harmony in diversity
diversity enriches culture
culture shapes identity
identity defines purpose
purpose gives meaning
meaning creates value
value drives decisions
decisions have consequences
consequences teach lessons
lessons learned grow wisdom
wisdom guides actions
actions speak louder
louder voices demand change
change is inevitable
inevitable progress continues
continues despite obstacles
obstacles build character
character reveals integrity
integrity earns trust
EOF

echo "✓ Created corpus with $(wc -l < large_corpus.txt) sentences"
echo

# Step 2: Pretrain embeddings
echo "Step 2: Pretraining word embeddings on large corpus..."
./gladtotext cbow \
  -input large_corpus.txt \
  -output pretrained_embeddings \
  -dim 100 \
  -epoch 15 \
  -minCount 1 \
  -lr 0.05 \
  -ws 5

echo "✓ Pretrained embeddings saved to pretrained_embeddings.bin"
echo

# Step 3: Create labeled classification data (small dataset)
echo "Step 3: Creating small labeled dataset..."
cat > classification_data.txt << 'EOF'
__label__positive wonderful and beautiful experience
__label__positive joy and happiness everywhere
__label__positive love and care for everyone
__label__positive quick and clever solution
__label__positive bright and promising future
__label__positive success and achievement
__label__positive peace and harmony
__label__positive trust and respect
__label__negative lazy and boring afternoon
__label__negative sad and difficult times
__label__negative slow and painful process
__label__negative bad and terrible news
__label__negative problems and obstacles
__label__negative wounds and suffering
__label__negative inevitable failure
__label__negative louder complaints
EOF

echo "✓ Created labeled dataset with $(wc -l < classification_data.txt) examples"
echo

# Step 4: Train classifier WITH pretrained embeddings
echo "Step 4: Training classifier WITH pretrained embeddings..."
./gladtotext supervised \
  -input classification_data.txt \
  -output classifier_with_pretrain \
  -pretrained pretrained_embeddings.bin \
  -epoch 25 \
  -lr 0.1

echo "✓ Classifier with pretrained embeddings saved"
echo

# Step 5: Train classifier WITHOUT pretrained embeddings (from scratch)
echo "Step 5: Training classifier WITHOUT pretrained embeddings (baseline)..."
./gladtotext supervised \
  -input classification_data.txt \
  -output classifier_from_scratch \
  -dim 100 \
  -epoch 25 \
  -lr 0.1 \
  -minCount 1

echo "✓ Classifier from scratch saved"
echo

# Step 6: Compare predictions
echo "=========================================="
echo "Step 6: Comparing Predictions"
echo "=========================================="
echo

test_cases=(
    "wonderful beautiful joy and happiness"
    "terrible sad and painful"
    "quick clever and bright"
    "lazy boring and slow"
    "success peace and harmony"
    "problems obstacles and failure"
)

for test in "${test_cases[@]}"; do
    echo "Test: '$test'"
    echo "  With pretrain:"
    echo "$test" | ./gladtotext-infer predict classifier_with_pretrain.bin 1 | head -1
    echo "  From scratch:"
    echo "$test" | ./gladtotext-infer predict classifier_from_scratch.bin 1 | head -1
    echo
done

echo "=========================================="
echo "Summary"
echo "=========================================="
echo
echo "Transfer learning workflow:"
echo "1. Pretrain embeddings on large unlabeled corpus (70+ sentences)"
echo "2. Fine-tune classifier on small labeled dataset (16 examples)"
echo "3. Benefit from semantic knowledge learned during pretraining"
echo
echo "Key advantages:"
echo "- Better word representations from large corpus"
echo "- Faster convergence on classification task"
echo "- Better generalization with limited labeled data"
echo "- Improved handling of word relationships"
echo
echo "Files created:"
echo "  - pretrained_embeddings.bin (pretrained word embeddings)"
echo "  - classifier_with_pretrain.bin (fine-tuned classifier)"
echo "  - classifier_from_scratch.bin (baseline classifier)"
echo
echo "Try your own predictions:"
echo "  echo 'your text here' | ./gladtotext-infer predict classifier_with_pretrain.bin 1"
