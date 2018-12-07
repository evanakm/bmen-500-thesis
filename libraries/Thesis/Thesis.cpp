#include "Arduino.h"
#include "Thesis.h"

Master::Master(uint8_t toSlave, uint8_t fromSlave, int pulseWidth_us){
	if (toSlave < 2 || toSlave > 7){
		throw "invalid toSlave pin";
	}
	if (fromSlave < 2 || fromSlave > 7){
		throw "invalid fromSlave pin";
	}
	if (toSlave == fromSlave){
		throw "toMaster and fromMaster must be different pins\n";
	}

	_pulseWidth_us = pulseWidth_us
	
	_toSlaveMask = 0b00000001 << toSlave;
	_fromSlaveMask = 0b00000001 << fromSlave;

	DDRB &= 0xc0; //set D13-D8 as inputs
	
	pinMode(MASTER_PIN_TO_SLAVE, OUTPUT);
	pinMode(MASTER_PIN_FROM_SLAVE, INPUT);
	
    PORTD &= ~_toSlaveMask; //Force output port low to start
	
	_state = MASTER_STATE_READY;
	_numberOfBytesSuccessfullyRead = 0;
}

void Master::enterWriteState(){
	_state = MASTER_STATE_COMMANDING;
	sendCommandToSlaveAndWaitForAcknowledgement(cmdWrite, _pulseWidth_us);
}

void Master::masterRequestAndReadOneByte(int delay_us){
	_state = MASTER_STATE_COMMANDING;
	sendCommandAndWaitForAcknowledgement(cmdRead1, delay_us);

	if(_state == MASTER_STATE_ACKNOWLEDGEMENT){
		sendSignalToSlaveAndReadFromBus(delay_us);
	}else{
		_state == MASTER_STATE_FAILED; //In theory this should be redundant, but in case I'm forgetting an edge case.
		return;
    }

	_state = MASTER_STATE_READY;
}

void Master::requestAndReadNBytes(int delay_us, uint8_t N){
	_numberOfBytesSuccessfullyRead = 0; //reset in case of failure
	if(N==0) return;

	state = MASTER_STATE_COMMANDING;
	masterSendCommandAndWaitForAcknowledgement(cmdReadN, delay_us);

	if(state == MASTER_STATE_ACKNOWLEDGEMENT){
		masterSendCommandAndWaitForAcknowledgement(N, delay_us);
	}
		else{
			_state == MASTER_STATE_FAILED;
		return;
	}
  
	sendSignalToSlaveAndReadFromBus(delay_us);
	
	_byteBuffer[0] = _readByte;
	
	if(N==1) {
		_numberOfBytesSuccessfullyRead = 1;
		return;
	}
  
	for(int i = 1; i < N; i++){
		synchronizedReadFromBus(delay_us);
		if (_state == MASTER_STATE_FAILED) return;
		
		//Could probably be optimized, but let's get it working first
		_byteBuffer[i] = _readByte;
	}
    _numberOfBytesSuccessfullyRead = N;
	_state = MASTER_STATE_WAITING;
}

void Master::resetBus(){
	sendPulseToSlave(_pulseWidth_us);
	delayMicroseconds(_pulseWidth_us);
	delayMicroseconds(_pulseWidth_us);
	sendCommandToSlaveAndWaitForAcknowledgement(cmdReset, _pulseWidth_us);
	
	//reset global variables
	_state = MASTER_STATE_WAITING;
	_numberOfBytesSuccessfullyRead = 0;
}

//Helper functions
uint8_t Master::readState(){
	return _state;
}

void Master::setToSlave(){
	PORTD |= _toSlaveMask;
}

void Master::resetToSlave(){
	PORTD &= ~_toSlaveMask;
}

void Master::takeControlOfBus(){
	DDRB |= 0x3f; //set D13-D8 as outputs
}

void Master::cedeControlOfBus(){
	DDRB &= 0xc0; //set D13-D8 as inputs
}

void Master::sendPulseToSlave(int delay_us){
	setToSlave();

	//for the sake of internal consistency, this is modeled as 2*delay_us
	delayMicroseconds(_pulseWidth_us);
	delayMicroseconds(_pulseWidth_us);
	
	resetToSlave();
}

void Master::sendCommandToSlaveAndWaitForAcknowledgement(cmdReset, _pulseWidth_us){
	setToSlave();

	//for the sake of internal consistency, this is modeled as 2*delay_us
	delayMicroseconds(_pulseWidth_us);
	delayMicroseconds(_pulseWidth_us);
	if(readFromSlave()){
		state = MASTER_STATE_ACKNOWLEDGEMENT;
	}
	else{
		state = MASTER_STATE_FAILED;
	}

	resetToSlave();
	delayMicroseconds(_pulseWidth_us);
	delayMicroseconds(_pulseWidth_us);	
}

bool Master::readFromSlave(){
	return _fromSlaveMask & PIND;
}

void Master::readByteFromBus(){
	_readByte = PINB;
}

uint8_t Master::accessReadByte(){
	return _readByte;
}

uint8_t* Master::accessByteBuffer(uint8_t &N){
	N = _numberOfBytesSuccessfullyRead;
	return _byteBuffer;
}

void Master::sendSignalToSlaveAndReadFromBus(int delay_us){
	sendPulseToSlave(delay_us);
	
	if(!readFromSlave()){
		state = MASTER_STATE_FAILED;
		return;
	}
	readByteFromBus();
	delay(delay_us);
	
	resetToSlave();
}

void Master::synchronizedReadFromBus(int delay_us){
	//The same as the above function, but for reading bytes of a string after the first one.
	delayMicroseconds(delay_us);
	if(fromSlave == 0){
		state = MASTER_STATE_FAILED;
		return;
	}
	readByteFromBus();
	delayMicroseconds(delay_us);
}

void Slave::Slave(uint8_t toMaster, uint8_t fromMaster){
	if (toMaster < 2 || toMaster > 7){
		throw "invalid toMaster pin\n";
	}
	if (fromMaster != 2 || fromMaster != 3){
		throw "invalid fromMaster pin\n";
	}
	if (toMaster == fromMaster){
		throw "toMaster and fromMaster must be different pins\n";
	}

	_pulseWidth_us = pulseWidth_us
	
	_toMasterMask = 0b00000001 << toSlave;
	_fromMasterMask = 0b00000001 << fromSlave;
	_state = SLAVE_STATE_READY;
	_counter = 0;
}

void Slave::writeOneByte(int delay_us){  
	PORTB &= (0xc0 | _byteToBeWritten);
	delayMicroseconds(delay_us);
	setToMaster();
	delayMicroseconds(delay_us);
	delayMicroseconds(delay_us);
	resetToMaster();
}

//N is length of dataArray. Size must be less than 64 (i.e. fit within 8 bits)
void Slave::writeNBytes(uint8_t N, int delay_us){
	if(N == 0) return;

	for(int i = 0; i < N; i++){
		if(state == SLAVE_STATE_INTERRUPTED){
			//cede control of the bus
			resetToMaster();
			state = SLAVE_STATE_READY;
			return;
		}
		PORTB &= (0xc0 | _bytesToBeWritten[i]);
		delayMicroseconds(delay_us);
		setToMaster();
		delay(delay_us);
		resetToMaster();
	}
}

//Helper functions
void Slave::setToMaster(){
	PORTD |= _toMasterMask;
}

void Slave::resetToMaster(){
	PORTD &= ~_toMasterMask;
}

void Slave::takeControlOfBus(){
	DDRB |= 0x3f; //set D13-D8 as outputs
}

void Slave::cedeControlOfBus(){
	DDRB &= 0xc0; //set D13-D8 as inputs
}


void Slave::reset(){
	resetToMaster();
	state = SLAVE_STATE_READY;
	counter = 0;
	//initialize any other global variables
}

void Slave::primeByteToBeWritten(uint8_t byteToBeWritten){
	_byteToBeWritten = byteToBeWritten;
}

//N is the number of bytes, must be 64 or less
void Slave::primeBytesToBeWritten(uint8_t* bytesToBeWritten, uint8_t N){
	if (N == 0) return;
	
	for(int i = 0; i < N; i++){
		_bytesToBeWritten[i] = bytesToBeWritten[i];
	}
}

void Slave::onRisingEdgeOfMasterToSlave(){
	switch(_state){
		case SLAVE_STATE_READY:
			cedeControlOfBus(); //Should be redundant, but get this working before removing
			mapCommandToState(0x3f & PINB);
			
			if(_state == SLAVE_STATE_ERROR) return;
			
			delayMicroseconds(_pulseWidth_us);
			setToMaster();
			
			delayMicroseconds(_pulseWidth_us);
			delayMicroseconds(_pulseWidth_us);

			resetToMaster();
			break;
		case SLAVE_STATE_PRIMED_TO_WRITE_ONE_BYTE:
			takeControlOfBus();
			writeOneByte(_pulseWidth_us);
			cedeControlOfBus();
			break;
		case SLAVE_STATE_AWAITING_NUMBER_OF_BYTES_TO_WRITE:
			cedeControlOfBus(); //Should be redundant, but get this working before removing
			_N = 0x3f & PINB;
			delayMicroseconds(_pulseWidth_us);
			setToMaster();
			delayMicroseconds(_pulseWidth_us);
			delayMicroseconds(_pulseWidth_us);
			resetToMaster();
			ReadAndPrimeNBytesOfData(_N);
			_state = SLAVE_STATE_PRIMED_TO_WRITE_N_BYTES;
			takeControlOfBus();
			break;
		case SLAVE_STATE_PRIMED_TO_WRITE_N_BYTES;
			_state = SLAVE_STATE_WRITING_TO_BUS;
			writeNBytes(_N,_pulseWidth_us);
			_state = SLAVE_STATE_READY;
			cedeControlOfBus();
			break;
		case SLAVE_STATE_WRITING_TO_BUS; //Interrupting
			_state = SLAVE_STATE_INTERRUPTED;
			cedeControlOfBus();
			break;
		case SLAVE_STATE_INTERRUPTED:
			mapCommandToState(0x3f & PINB);
			break;
			
	}
}

void Slave::mapCommandToState(uint8_t cmd){
	switch(cmd){
		case cmdWrite:
			_state = SLAVE_STATE_READING;
			break;
		case cmdRead1:
		    ReadAndPrimeOneByteOfData();
			_state = SLAVE_STATE_PRIMED_TO_WRITE_ONE_BYTE;
			break;
		case cmdReadN:
			_state = SLAVE_STATE_AWAITING_NUMBER_OF_BYTES_TO_WRITE;
			break;
		case cmdReset:
			_state = SLAVE_STATE_RESETTING;
			break;
		default:
			_state = SLAVE_STATE_ERROR;
			break;
	}

}

void Slave::readAndPrimeOneByteOfData(){
	primeByteToBeWritten(_counter);
	counter = counter + 1 % 64;
}

//Precondition: N <= 64
void Slave::readAndPrimeNBytesOfData(uint8_t N){
	if (N==0) return;
		
	uint8_t bytes[N];
	
	for(int i = 0; i < N; i++){
		bytes[i] = _counter;
		counter = _counter + 1 % 64;
	}
	
	primeBytesToBeWritten(bytes, N);
	
}