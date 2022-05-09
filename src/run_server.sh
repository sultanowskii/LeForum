#!/bin/bash

./LeForumServer \
        --host=0.0.0.0 \
        --port=7431 \
        --hello-message="Have a nice day :)" \
        --timeout=3 \
        --max-connections=100
