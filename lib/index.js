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

function cpuTemperature(callback) {

  return new Promise((resolve, reject) => {
    process.nextTick(() => {
      if (_windows) {
        let error = new Error(NOT_SUPPORTED);
        if (callback) { callback(NOT_SUPPORTED) }
        reject(error);
      }
      if (_linux) {
        let error = new Error(NOT_SUPPORTED);
        if (callback) { callback(NOT_SUPPORTED) }
        reject(error);
      }

      if (_darwin) {
        let result = {
          main: -1.0,
          cores: [],
          max: -1.0
        };
        let smc = require('../build/Release/smc');

        let cores = ['TC0P', 'TC1C', 'TC2C', 'TC3C', 'TC4C', 'TC5C', 'TC6C', 'TC7C', 'TC8C'];
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

        if (callback) { callback(result) }
        resolve(result);
      }
    });
  });
}
exports.version = version;
exports.cpuTemperature = cpuTemperature;
