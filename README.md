# jsonstreamevents 
Cross-platform C library to parse JSON files while calling callback functions.

A callback function can be specified for the following parsing events:
- begin of an object or an array
- end of an object or an array
- value

## prerequisites
JSON parsing itself is done by one of the following libraries:
- [yajl](https://github.com/lloyd/yajl/)
- [liblaxjson](https://github.com/andrewrk/liblaxjson)
