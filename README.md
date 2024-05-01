# jsonstreamevents 
Cross-platform C library to parse JSON files while calling callback functions.

## Goal
The goal of this library is to be able to parse a JSON file without loading them into memory.
The JSON data is parsed in chunks as they become available and callback functions are called for specific events.

A callback function can be specified for the following parsing events:
- begin of an object or an array
- end of an object or an array
- value

## Prerequisites
JSON parsing itself is done by one of the following libraries:
- [yajl](https://github.com/lloyd/yajl/)
- [liblaxjson](https://github.com/andrewrk/liblaxjson)
