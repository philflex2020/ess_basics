 build/release/dnp3_server config/test_nontls_server.json&
sleep 1
sh scripts/dnp3_tls_server_echo.sh
