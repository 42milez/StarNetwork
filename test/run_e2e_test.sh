#!/bin/bash

/var/app/build/test/e2e/rudp/basic_connection/basic_connection_test           -r console -d yes --order lex
/var/app/build/test/e2e/rudp/broadcast/broadcast_test                         -r console -d yes --order lex
/var/app/build/test/e2e/rudp/send_reliable_command/send_reliable_command_test -r console -d yes --order lex
