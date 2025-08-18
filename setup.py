from setuptools import setup, Extension
from pybind11.setup_helpers import Pybind11Extension, build_ext
from pybind11 import get_cmake_dir
import pybind11
import os

class get_pybind_include(object):
    """Helper class to determine the pybind11 include path

    The purpose of this class is to postpone importing pybind11
    until it is actually installed, so that the ``get_include()``
    method can be invoked. """

    def __str__(self):
        import pybind11
        return pybind11.get_include()


def get_extensions():
    # Define the extension module
    ext_modules = [
        Pybind11Extension(
            "pyrdma",
            [
                "src/pyrdma.cpp",
                "src/tcp_communicator.cpp",
                "src/rdma_communicator.cpp",
            ],
            include_dirs=[
                "src/",
                get_pybind_include(),
                "/usr/include/",
            ],
            libraries=["ibverbs"],
            library_dirs=["/usr/lib/x86_64-linux-gnu/"],
            cxx_std=11,
            extra_compile_args=["-Wall", "-Wextra", "-g", "-fPIC"],
        ),
    ]
    return ext_modules


# Read the README file for long description
with open("README.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()

setup(
    name="pyrdma",
    version="1.0.0",
    author="Your Name",
    author_email="your.email@example.com",
    description="A Python wrapper for RDMA communication using pybind11",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/yourusername/pyrdma",
    ext_modules=get_extensions(),
    setup_requires=["pybind11"],
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    python_requires=">=3.6",
    classifiers=[
        "Development Status :: 4 - Beta",
        "Intended Audience :: Developers",
        "Topic :: Software Development :: Libraries :: Python Modules",
        "License :: OSI Approved :: MIT License",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
    ],
    packages=[],
)