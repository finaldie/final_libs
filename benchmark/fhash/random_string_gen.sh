#!/bin/sh

timeout 20s strings /dev/urandom | base64
