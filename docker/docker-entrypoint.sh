#!/bin/sh
#set -e

regenerate_dbus_machine_id() {
    rm -f /var/lib/dbus/machine-id
    rm -f /etc/machine-id
    dbus-uuidgen --ensure
}

CONTAINER_ALREADY_STARTED="CONTAINER_ALREADY_STARTED_PLACEHOLDER"
if [ ! -e $CONTAINER_ALREADY_STARTED ]; then
    touch $CONTAINER_ALREADY_STARTED
	regenerate_dbus_machine_id
else
    echo "machine-id already was regenerated"
fi

if [ "${1#-}" != "$1" ]; then
	set -- fastocloud "$@"
fi

exec "$@"
