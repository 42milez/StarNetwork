#!/bin/bash

cd /var/app/build || exit

/usr/bin/cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -G "CodeBlocks - Unix Makefiles" /var/app
/usr/bin/cmake --build /var/app/build --target all -- -j 4

/var/app/build/test/e2e/rudp/basic_connection/basic_connection_test -r console -d yes --order lex
/var/app/build/test/e2e/rudp/broadcast/broadcast_test -r console -d yes --order lex
/var/app/build/test/e2e/rudp/packet_loss/packet_loss_test -r console -d yes --order lex
/var/app/build/test/e2e/rudp/send_reliable_command/send_reliable_command_test -r console -d yes --order lex
