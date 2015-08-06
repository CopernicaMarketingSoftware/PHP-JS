#!/bin/sh

# Disable grep options
unset GREP_OPTIONS

# do we have a project number or version directory?
version=$(git tag | tail -n 1)

# which one is set?
if [ ! -z $version ]; then
    echo $version | cut --delimiter=v --fields=2
else
    echo "dev"
fi
