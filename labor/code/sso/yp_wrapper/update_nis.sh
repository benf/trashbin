#!/bin/bash
kinit -k $(hostname -s)
DIR=$PWD

cd /var/yp
/root/ldap_con objectclass=user > passwd.ldap
/root/ldap_con objectclass=group > group.ldap

make
cd $DIR
