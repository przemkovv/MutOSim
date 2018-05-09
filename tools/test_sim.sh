#!/bin/sh

OUTPUT_DIR=tests/output
ORIGIN_DIR=tests/origin
ARGS="--start=1.1 --stop=1.2 --step=0.1 --count=1 --random=0 --duration=300000 --parallel=0 -d $OUTPUT_DIR -q 0"
SCENARIOS_DIR=data/scenarios

SCENARIOS=(simple_erlang.json simple_engset.json simple_pascal.json single_overflow_eng_erl_pas.json)

RED='\033[0;31m'
BLUE='\033[0;34m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

TIME_FORMAT="%U user %S system %E elapsed %P CPU"
for SCENARIO in ${SCENARIOS[@]}; do
  echo -e "Scenario ${BLUE}$SCENARIO${NC}"
  time -f "$TIME_FORMAT" -o $OUTPUT_DIR/${SCENARIO}_time build/bin/sim -f $SCENARIOS_DIR/$SCENARIO $ARGS -o $SCENARIO 2>&1 1>/dev/null
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

  if [ ! $diffRetVal  -eq 0 ]; then
    break
  fi
  echo ""
done
