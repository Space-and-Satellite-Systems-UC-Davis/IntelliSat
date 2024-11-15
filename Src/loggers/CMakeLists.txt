cmake_minimum_required(VERSION 3.10)

project(IntellisatLoggers VERSION 0.1.0 LANGUAGES C)

set(LOGGERS_SOURCES
    event_logger.c
    experiment_logger.c
    fits_in_bits.c
    downlink_logs.c
    logger_to_flash.c
)

set( MOCK_FLASH_SOURCES
    mockup_flash/mock_flash_driver.c
)

add_executable(IntellisatLoggers
    ${LOGGERS_SOURCES}
    ${MOCK_FLASH_SOURCES}
    loggers_main.c
)

add_library(IntellisatLoggersLib
    ${LOGGERS_SOURCES}
    ${MOCK_FLASH_SOURCES}
)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

target_include_directories(IntellisatLoggers PUBLIC ./ ./mockup_flash)
target_include_directories(IntellisatLoggersLib PUBLIC ./ ./mockup_flash)

target_compile_definitions(IntellisatLoggersLib PUBLIC NO_OP)
