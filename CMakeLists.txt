cmake_minimum_required( VERSION 3.5 )

include( CTest )

set( teos $ENV{EOSIO_EOSFACTORY_DIR}/teos/build/teos/teos )
add_custom_target( abi ALL COMMAND ${teos} generate abi ${CMAKE_SOURCE_DIR}/src/contracts/Lottery )
add_custom_target( wast ALL COMMAND ${teos} build contract ${CMAKE_SOURCE_DIR}/src/contracts/Lottery )

add_test( NAME lottery.spec COMMAND python3 ${CMAKE_SOURCE_DIR}/test/lottery.spec.py )

# ctest -V -R ^lottery.spec$

