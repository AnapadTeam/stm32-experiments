#!/bin/bash

cd "$(dirname "$0")" # cd to script directory
cd ../../ # 'git apply' must be called from Git root
git apply HIDKeyboardHacker/patch/usbd_hid.patch
