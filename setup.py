from setuptools import setup, Extension
import sys
import os

# Read version from a single source
__version__ = "0.1.0"

# Read long description from README
try:
    with open("README.md", "r", encoding="utf-8") as fh:
        long_description = fh.read()
except FileNotFoundError:
    long_description = "GLADtoTEXT - FastText-inspired text embeddings with sentence-level encoding"

# Import pybind11 after it's been installed as a setup requirement
try:
    from pybind11.setup_helpers import Pybind11Extension, build_ext
    
    ext_modules = [
        Pybind11Extension(
            "gladtotext",
            ["python_bindings.cpp"],
            include_dirs=["."],
            cxx_std=17,
            extra_compile_args=["-O3", "-Wall", "-Wextra"],
        ),
    ]
    cmdclass = {"build_ext": build_ext}
except ImportError:
    # Fallback if pybind11 not available yet
    ext_modules = [
        Extension(
            "gladtotext",
            ["python_bindings.cpp"],
            include_dirs=["."],
            extra_compile_args=["-std=c++17", "-O3", "-Wall", "-Wextra"],
        ),
    ]
    cmdclass = {}

setup(
    name="gladtotext",
    version=__version__,
    author="github@vksssd",
    author_email="",
    description="FastText-inspired text embeddings with sentence-level encoding",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/vksssd/GLADtoTEXT",
    project_urls={
        "Bug Tracker": "https://github.com/vksssd/GLADtoTEXT/issues",
        "Documentation": "https://github.com/vksssd/GLADtoTEXT#readme",
        "Source Code": "https://github.com/vksssd/GLADtoTEXT",
    },
    ext_modules=ext_modules,
    cmdclass=cmdclass,
    setup_requires=[
        "pybind11>=2.10.0",
    ],
    zip_safe=False,
    python_requires=">=3.7",
    install_requires=[
        "numpy>=1.19.0",
    ],
    extras_require={
        "dev": [
            "pytest>=6.0",
            "black>=22.0",
            "build>=0.7.0",
            "twine>=3.4.0",
        ],
    },
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Developers",
        "Intended Audience :: Science/Research",
        "Topic :: Scientific/Engineering :: Artificial Intelligence",
        "Topic :: Text Processing :: Linguistic",
        "License :: OSI Approved :: MIT License",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: Python :: 3.12",
        "Programming Language :: C++",
        "Operating System :: OS Independent",
    ],
    keywords="nlp embeddings text-classification fasttext sentence-encoding",
)
