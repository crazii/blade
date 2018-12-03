if [ -f /etc/DIR_COLORS ] && cmp -s /etc/defaults/etc/DIR_COLORS /etc/DIR_COLORS
then
    rm /etc/DIR_COLORS
fi

