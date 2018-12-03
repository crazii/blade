if [ -f /etc/man_db.conf ] && cmp -s /etc/defaults/etc/man_db.conf /etc/man_db.conf
then
    rm /etc/man_db.conf
fi

