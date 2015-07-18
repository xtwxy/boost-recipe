# !/bin/sh

export LANG=en_US.utf8
mount -t nfs -o rw,nolock 192.168.0.78:/nfsshare/lib /lib
mount -t nfs -o rw,nolock 192.168.0.78:/nfsshare/usr /usr
