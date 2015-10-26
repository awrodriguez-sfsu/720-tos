#!/usr/bin/env bash
make clean
if(make tests); then
  java -jar ttc.jar
  make clean
  clear
  clear
fi
make clean