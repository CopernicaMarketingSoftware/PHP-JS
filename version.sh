#!/bin/sh

# Disable grep options
unset GREP_OPTIONS

# do we have a project number or version directory?
project=$(pwd -P | grep -o -e "[#][[:digit:]]\+")
version=$(pwd -P | grep -o -e "PxJavascript/[[:digit:]]\+[.][[:digit:]]\+[.]\?[[:digit:]]\?")

# which one is set?
if [ ! -z $project ]; then
    echo "project $project"
elif [ ! -z $version ]; then
    echo $version | cut --delimiter=/ --fields=2
else
    echo "dev"
fi
