#!/bin/bash
grep -o "^can.* in [0-9]\+" | sed -e "s/can't solve /- /" -e "s/can solve /+ /" -e "s/ size=[0-9/]* / /" -e "s/Sa(/a /" -e "s/) in / /" -e "s/Sb(/b /" -e "s/)\[/ t /" -e "s/] in / /" -e "s/]//" -e "s/[:,]/ /g"

