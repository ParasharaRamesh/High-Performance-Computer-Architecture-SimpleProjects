# Compile
make -f Makefile

# Image and param to use
img_def='big'
kernel_def='edge'
passes=10
if [[ -z $1 ]]; then img_in="test/img_in/${img_def}.png"; else img_in=$1; fi
if [[ -z $2 ]]; then kernel="test/img_kernel/${kernel_def}.txt"; else kernel=$2; fi
if [[ -z $3 ]]; then img_out="test/img_out/${img_def}_${kernel_def}.png"; else img_out=$3; fi

# Run stuff
print() {
  echo "--- $1 ---"
}

print 'Running sequential'
time ./build/sequential $img_in $kernel $passes $img_out
print 'Done sequential'
echo ''

print 'Running parallel'
time ./build/parallel $img_in $kernel $passes $img_out
print 'Done parallel'
echo ''

print 'Running parallel (over-CPU)'
time OMP_NUM_THREADS=10 ./build/parallel $img_in $kernel $passes $img_out
print 'Done parallel over'
echo ''

echo 'Done!'
