#!/bin/bash

"${HOME}"/build/test/e2e/rudp/basic_connection/basic_connection_test -r console -d yes --order lex
"${HOME}"/build/test/e2e/rudp/broadcast/babroadcast_testsic_connection_test -r console -d yes --order lex
"${HOME}"/build/test/e2e/rudp/send_reliable_command/send_reliable_command_test -r console -d yes --order lex
