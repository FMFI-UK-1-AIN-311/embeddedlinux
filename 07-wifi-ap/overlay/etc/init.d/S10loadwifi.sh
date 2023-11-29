#!/bin/bsh

case "$1" in
        start|stop|restart|reload)
                modprobe brcmfmac
                ;;
        *)
                echo "Usage: $0 {start|stop|restart|reload}"
                exit 1
                ;;
esac

