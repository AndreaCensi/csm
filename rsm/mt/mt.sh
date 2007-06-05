#!/bin/bash

inl=../../../experiments/mbicp_tro_experiment/laserazosSM3.log

ruby -I.. -rrubygems -rsm -rsm_icp mt.rb Sm::ICPC '[]' $inl out.log 2>&1
