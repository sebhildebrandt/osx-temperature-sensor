'use strict';
// ==================================================================================
// index.js
// ----------------------------------------------------------------------------------
// Description:   osx temperature - library
//                for Node.js
// Copyright:     (c) 2017
// Author:        Sebastian Hildebrandt
// ----------------------------------------------------------------------------------
// Contributors:
// ----------------------------------------------------------------------------------
// License:       MIT
// ==================================================================================
//

const lib_version = require('../package.json').version;

const os = require('os');

let _platform = os.type();

const _linux = (_platform === 'Linux');
const _darwin = (_platform === 'Darwin');
const _windows = (_platform === 'Windows_NT');
const NOT_SUPPORTED = 'not supported';

function version() {
  return lib_version;
}

// --------------------------
// CPU - temperature
// if sensors are installed

function cpuTemperature() {
  if (_windows) {
    return(NOT_SUPPORTED);
  }
  if (_linux) {
    return(NOT_SUPPORTED);
  }

  if (_darwin) {
    let result = {
      main: -1.0,
      cores: [],
      max: -1.0
    };
    let smc = require('../build/Release/smc');

    let cores = [];
    if (process.arch == 'arm64') {
      cores = ['Tp01', 'Tp05', 'Tp0D', 'Tp0H', 'Tp0L', 'Tp0P', 'Tp0X', 'Tp0b', 'Tp09', 'Tp0T'];
    } else {
      cores = ['TC0P', 'TC1C', 'TC2C', 'TC3C', 'TC4C', 'TC5C', 'TC6C', 'TC7C', 'TC8C'];
    }

    let sum = 0;
    let id = 0;
    cores.forEach(function(key) {
      let value = smc.get(key);
      if (id === 0) {
        if (value > 0) {
          result.main = value;
          result.max = value;
        }
        id = 1;
      } else {
        if (value > 0) {
          result.cores.push(value);
          sum = sum + value;
          if (value > result.max) result.max = value;
        }
      }
    });
    if (result.cores.length) {
      result.main = sum / result.cores.length;
    }

    return(result);
  }
}
exports.version = version;
exports.cpuTemperature = cpuTemperature;
