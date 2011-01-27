#!/bin/bash
set -e
set -x
git clone git@github.com:AndreaCensi/csm.git  output
cd output
git checkout origin/gh-pages -b gh-pages
git branch -D master

