#!/bin/bash

#  Copyright (C) 2018-2020 LEIDOS.
# 
#  Licensed under the Apache License, Version 2.0 (the "License"); you may not
#  use this file except in compliance with the License. You may obtain a copy of
#  the License at
# 
#  http://www.apache.org/licenses/LICENSE-2.0
# 
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
#  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
#  License for the specific language governing permissions and limitations under
#  the License.

# CARMA packages checkout script
# Optional argument to set the root checkout directory with no ending '/' default is '~'

set -ex

dir=~
while [[ $# -gt 0 ]]; do
      arg="$1"
      case $arg in
            -d|--develop)
                  BRANCH=develop
                  shift
            ;;
            -r|--root)
                  dir=$2
                  shift
                  shift
            ;;
      esac
done

cd ${dir}/src

if [[ "$BRANCH" = "develop" ]]; then
      git clone --depth=1 https://github.com/usdot-fhwa-stol/CARMAMsgs.git --branch $BRANCH
      git clone --depth=1 https://github.com/usdot-fhwa-stol/CARMANovatelGpsDriver.git --branch $BRANCH
      git clone --depth=1 https://github.com/usdot-fhwa-stol/CARMAUtils.git --branch $BRANCH
      git clone --depth=1 https://github.com/usdot-fhwa-stol/carma-messenger.git --branch $BRANCH
else
      git clone --depth=1 https://github.com/usdot-fhwa-stol/CARMAMsgs.git --branch develop
      git clone --depth=1 https://github.com/usdot-fhwa-stol/CARMANovatelGpsDriver.git --branch CARMANovatelGpsDriver_1.2.0
      git clone --depth=1 https://github.com/usdot-fhwa-stol/CARMAUtils.git --branch develop
      git clone --depth=1 https://github.com/usdot-fhwa-stol/carma-messenger.git --branch develop
fi
