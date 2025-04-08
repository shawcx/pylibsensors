#!/usr/bin/env python3

import sys
import platform

import setuptools

defines = []
include_dirs = []
library_dirs = []
libs = ['sensors']

setuptools.setup(
    name             = 'pylibsensors',
    version          = '0.1',
    author           = 'Matthew Shaw',
    author_email     = 'mshaw.cx@gmail.com',
    license          = 'MIT',
    description      = 'Python3 bindings for libsensors',
    long_description = open('README.rst').read(),
    packages         = setuptools.find_packages(),
    ext_modules = [
        setuptools.Extension(
            'pylibsensors',
            ['pylibsensors.c'],
            define_macros = defines,
            include_dirs = include_dirs,
            library_dirs = library_dirs,
            libraries = libs
            )
        ],
    classifiers = [
        'Development Status :: 3 - Alpha',
        'Intended Audience :: Developers',
        'License :: Proprietary',
        'Programming Language :: Python :: 3',
        ],
    )
