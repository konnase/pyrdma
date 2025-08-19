# pyrdma

## 1. Prerequisites

- **libibverbs-dev** (for RDMA operations)
- **CMake** (recommended build system)
- **Python 3.6+** (for Python bindings)
- **pybind11** (for Python bindings)

Install dependencies on Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install -y libibverbs-dev cmake python3-dev
```

## 2. Installation

### Build from Source (C++)
```bash
bash hack/build.sh
```

### Python Package Installation
```bash
# Build wheel package
bash hack/build_whl.sh

# Install package
pip install dist/pyrdma-*.whl

# Or development mode
pip install -e .
```

## 3. Example

### Python Usage
Refer to [examples/test_pyrdma.py](examples/test_pyrdma.py)

### C++ Usage
Refer to [examples/rdma](examples/rdma) and [examples/tcp](examples/tcp)

## 4. Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

For questions or issues, please open a GitHub issue.
