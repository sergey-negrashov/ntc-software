#!/bin/bash

mkdir  -p DRS4-util/drs_ntc/proto
cd proto
protoc ntc.proto --cpp_out=../DRS4-util/drs_ntc/proto
protoc ntc.proto --python_out=../dog_ang_pony_show
cd ..
