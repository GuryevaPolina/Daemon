# Daemon #

The simplest daemon which remove all files older then one minute.

# Instruction #
1. Add full path to directory in daemon.c (to "pathToCurrentDirectory" variable, 16 line)
2. Add full path to directory in test.sh (after "cd", 1 line)
3. Call in cmd:
```
make
./daemon start
sh test.sh
```

