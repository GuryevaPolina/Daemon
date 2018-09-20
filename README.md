# Simple Daemon #

The simplest daemon which does nothing.

## How to make sure it works ##

```bash
make
./daemon
ps -axj | grep ./daemon
```

## Have to be implemented ##

* The other signals handling
* Support for command line arguments: stop and start.
  ```./daemon start``` should check if the daemon is running and start it if it's not.
  ```./daemon stop``` should check if the daemon is running and stop it if it is.
