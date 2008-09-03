#!/bin/bash

echo '[0,0,0]' | ./raytracer -map test01-map.json  > test01-scans.json
diff test01-scans.json{,.expected}
