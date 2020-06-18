#!/bin/bash
for mdfile in examples/*.md ; do
    autoproject "$mdfile"
done
