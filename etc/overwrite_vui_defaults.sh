#!/bin/bash
set -e
echo "Deleting original files"
rm ~/Vrui-3.0/etc/Control.cfg
rm ~/Vrui-3.0/etc/OculusRift.cfg
rm ~/Vrui-3.0/etc/RazerHydra.cfg
rm ~/Vrui-3.0/etc/VRDevices.cfg
rm ~/Vrui-3.0/etc/Vrui.cfg
echo "Creating hard links"
ln Control.cfg ~/Vrui-3.0/etc/Control.cfg
ln OculusRift.cfg ~/Vrui-3.0/etc/OculusRift.cfg
ln RazerHydra.cfg ~/Vrui-3.0/etc/RazerHydra.cfg
ln VRDevices.cfg ~/Vrui-3.0/etc/VRDevices.cfg
ln Vrui.cfg ~/Vrui-3.0/etc/Vrui.cfg
echo "Done"