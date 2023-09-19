cmake_minimum_required(VERSION 3.18)
project(fsync_channel C)

set(CMAKE_C_STANDARD 11)

add_executable(receiver receiver.c)

add_executable(sender sender.c)
