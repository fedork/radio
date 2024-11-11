#!/bin/bash
cd ~/radio
tail -n +1 -f `ls -t ~/radio/pareto9_*.txt | head -1` | grep --color "result\|still\|took [0-9]\{3,\}\|in [891]"
