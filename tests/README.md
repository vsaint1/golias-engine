# Testing Guide for Golias Engine

## Building and Running Tests

### Build Tests
```bash
cmake --build --preset tests 

cmake --build --preset tests-build
```

### Run All Tests
```bash
ctest --test-dir build/default/ --gtest_color=1
```
