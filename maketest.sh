#!/bin/bash
docker run --rm -v $PWD/:/home/user/9cc -w /home/user/9cc compilerbook make test
