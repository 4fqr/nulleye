#!/usr/bin/env bash
set -euo pipefail
cc -std=gnu11 -Wall -Wextra -I.. test_event_bus.c ../core/event_bus.c -o test_event_bus
./test_event_bus

cc -std=gnu11 -Wall -Wextra -I.. test_file_integrity.c ../utils/hash.c ../core/db.c ../core/logger.c ../core/config.c -lsqlite3 -lcrypto -o test_file_integrity
./test_file_integrity

cc -std=gnu11 -Wall -Wextra -I.. test_lstm.c ../ai/lstm_predictor.c -lm -o test_lstm
./test_lstm

cc -std=gnu11 -Wall -Wextra -I.. test_rules.c ../core/rules.c ../core/response_engine.c ../core/logger.c ../core/db.c ../core/config.c -lsqlite3 -o test_rules
./test_rules

echo "All tests passed"