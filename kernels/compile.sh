#!/bin/bash

glslang -V ma.comp -o ma.spv
glslang -V branch.comp -o branch.spv
glslang -V heavy.comp -o heavy.spv
