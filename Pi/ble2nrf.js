'use strict';

const minimist = require('minimist'); //Put this in the package.json file
const noble = require('noble');

const NRF_UUID = '1207'

// specify the services and characteristics to discover
const serviceUUIDs = [NRF_UUID];
const characteristicUUIDs = [NRF_UUID];

let args = minimist(process.argv.slice(2), {  
    alias: {
        h: 'help',
        v: 'version'
    }
});

let valueToSet = args.set ? args.set : 0;

noble.on('stateChange', state => {
  if (state === 'poweredOn') {
    console.log('Scanning');
    noble.startScanning([NRF_UUID]);
  } else {
    noble.stopScanning();
  }
});

noble.on('discover', peripheral => {
    // connect to the first peripheral that is scanned
    noble.stopScanning();
    const name = peripheral.advertisement.localName;
    console.log(`Connecting to '${name}' ${peripheral.id}`);
    connectAndSetUp(peripheral);
    
    peripheral.on('disconnect', () => {
        console.log('disconnected');
    });
	
	peripheral.on('disconnect', () => {
        console.log('disconnected');
    });
    
});

function connectAndSetUp(peripheral) {

  peripheral.connect(error => {
    console.log('Connected to', peripheral.id);
    peripheral.discoverServices(serviceUUIDs, onServiceDiscovered);  
  })
  
};

function onServiceDiscovered(error, service) {
  console.log('Discovered service');

  let len = service.length;

  let arr = new Uint8Array(1);
  arr[0] = valueToSet;
  let setter = Buffer.from(arr);
  
  service[0].discoverCharacteristics(characteristicUUIDs, function(error,characteristic) {
    characteristic[0].write(setter, false);
  });
  
};
