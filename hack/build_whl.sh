BASE_DIR=$(dirname "$0")/..
cd $BASE_DIR

# Build the wheel
python3 setup.py sdist bdist_wheel
