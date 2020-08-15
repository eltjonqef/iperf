# iperf

# Usage:
```sh
$ make
```
## Server:
```sh
$ ./server [mandatory] [options]
```

### Mandatory:
-a       Server IP  
-p       Server Port
### Options:
-i       Print Interval  
-f       Output File

## Client:
```sh
$ ./client [mandatory] [options]
```

### Mandatory:
-a       Server IP  
-p       Server Port  
-l       Experiment Packet Size  
-b       Bandwidth

### Options
-n       Number of parallel streams  
-t       Experiment Duration  
-d       One way delay  
-w       Wait duration