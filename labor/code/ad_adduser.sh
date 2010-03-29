#!/bin/bash
_USER=banzai
_GROUP=gBANZAI
_PASSWD="******"
_UID="2007"
_GID="2007"
_NIS_DOMAIN="lab"
_SHELL="/bin/bash"

HOME_PREFIX="/home/users"
OU_NAME=nsmlab

HOMEDIR_SERVER=huntin


OU="$(adtool search ou $OU_NAME)"

adtool usercreate   $_USER  $OU
adtool setpass      $_USER  $_PASSWD
adtool groupcreate  $_GROUP $OU
adtool groupadduser $_GROUP $_USER

adtool attributereplace $_USER "homeDirectory" '\\ostseestadion\Homes\'${USER}'\'${USER}
adtool attributereplace $_USER "msSFU30Name" _USER
adtool attributereplace $_USER "msSFU30NisDomain" $_NIS_DOMAIN
adtool attributereplace $_USER "uidNumber" $_UID
adtool attributereplace $_USER "gidNumber" $_GID
adtool attributereplace $_USER "unixHomeDirectory" "${HOME_PREFIX}/$_USER/$_USER/"
adtool attributereplace $_USER "loginShell" $_SHELL

adtool attributereplace $_GROUP "gidNumber" $_GID
adtool attributereplace $_GROUP "msSFU30Name" $_USER
adtool attributereplace $_GROUP "msSFU30NisDomain" $_NIS_DOMAIN


adtool userunlock $_USER

ssh $HOMEDIR_SERVER "mkdir -p /export/users/$_USER/$_USER/"
ssh $HOMEDIR_SERVER "chown -R $_UID:$_GID /export/users/$_USER/$_USER/"

