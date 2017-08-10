#!/bin/bash

export PE_RANK=$ALPS_APP_PE export PMI_NO_FORK=1
if [ "$PE_RANK" == "0" ];then
	echo "rank 0"
	/opt/intel/vtune_amplifier_xe/bin64/amplxe-cl -collect advanced-hotspots -analyze-system -start-paused  -finalization-mode=none ./bin/composite_knl_perf
fi
