#!/bin/sh

# set -x

OUTPUT_DIR=data/tests/output
ORIGIN_DIR=data/tests/origin
ARGS="-m simulation -m analytic --analytic_model=KRFixedReqSize --analytic_model=KRFixedCapacity -c1 --start 0.4 --stop 1.8 --step 0.1 -t 1000 --parallel=true -r0  -d $OUTPUT_DIR"
# ARGS="--start=1.1 --stop=1.2 --step=0.1 --count=1 --random=0 --duration=300000 --parallel=0 -d $OUTPUT_DIR -q 0"
SCENARIOS_DIR=data/scenarios/analytical

# SCENARIOS=(simple_erlang.json simple_engset.json simple_pascal.json single_overflow_eng_erl_pas.json)
SCENARIOS=(const_ratio/3_1g_126_60.json const_ratio/3_1g_126_600.json const_ratio/3_1g_126_6000.json const_ratio/3_1g_126_60000.json var_ratio/3_1g_126_60.json var_ratio/3_1g_126_600.json var_ratio/3_1g_126_6000.json 1ov/3_1g_126_60.json 3ov/3_1g_126_60.json  )


RED='\033[0;31m'
BLUE='\033[0;34m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

TIME_FORMAT="%U user %S system %E elapsed %P CPU"
for SCENARIO in ${SCENARIOS[@]}; do
  echo -e "Scenario ${BLUE}$SCENARIO${NC}"
  time -f "$TIME_FORMAT" -o $OUTPUT_DIR/${SCENARIO}_time ../mutosim_build/bin/mutosim -f $SCENARIOS_DIR/$SCENARIO $ARGS -o $SCENARIO 2>&1 1> $OUTPUT_DIR/${SCENARIO}_log
  diff $OUTPUT_DIR/$SCENARIO $ORIGIN_DIR/$SCENARIO
  diffRetVal=$?
  if [ ! $diffRetVal -eq 0 ]; then
    echo -e "Diff ${RED}ERROR${NC}"
  else
    echo -e  "Diff ${GREEN}OK${NC}"
  fi

  echo -n "Time before: "
  cat $OUTPUT_DIR/${SCENARIO}_time
  echo -n "Time after:  "
  cat $ORIGIN_DIR/${SCENARIO}_time

  # if [ ! $diffRetVal  -eq 0 ]; then
    # exit 1
  # fi
  echo ""
done
