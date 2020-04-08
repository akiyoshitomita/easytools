#!/bin/bash

ffprobe -i $1 -show_streams -show_format -print_format json -loglevel quiet
