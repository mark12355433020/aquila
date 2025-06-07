#!/bin/zsh

rm -rf ./staging
rm -rf ./payload.dmg

mkdir -p ./staging/Library/Lockdown/ServiceAgents
cp -a ./haxx.aquila.remount.plist ./staging/Library/Lockdown/ServiceAgents/haxx.aquila.remount.plist
cp -a ./haxx.aquila.afc2.plist ./staging/Library/Lockdown/ServiceAgents/haxx.aquila.afc2.plist
cp -a ./haxx.aquila.housekeeping.plist ./staging/Library/Lockdown/ServiceAgents/haxx.aquila.housekeeping.plist

chmod -R 0777 staging
find ./staging -name ".DS_Store" -delete

hdiutil create -srcfolder staging -volname "root" -fs HFS+ -format UDRW -imagekey bps=512 \
    -imagekey appendable=true -imagekey burnable=true -ov -layout NONE ./payload.dmg
