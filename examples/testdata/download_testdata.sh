#!/bin/sh
curl -L -H "Accept: application/vnd.github+json" -H "X-GitHub-Api-Version: 2022-11-28" -k https://api.github.com/repos/brechtsanders/jsonstreamevents > projectinfo.json
curl -L -H "Accept: application/vnd.github+json" -H "X-GitHub-Api-Version: 2022-11-28" -k https://api.github.com/repos/brechtsanders/jsonstreamevents/contents/ > projectfiles.json
