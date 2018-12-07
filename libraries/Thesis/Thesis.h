// Originally written November 2018
// by Evan Meikleham

#ifndef _THESIS_H_
#define _THESIS_H_

// Commands for both Master and Slave

#define cmdReset 0x02
#define cmdWrite 0x03
#define cmdRead1 0x04
#define cmdReadN 0x05

// Master states. The two MSB must be zero since the bus is six bits.

#define MASTER_STATE_READY 0x00
#define MASTER_STATE_COMMANDING 0x02
#define MASTER_STATE_WRITE 0x03
#define MASTER_STATE_READ_1 0x04
#define MASTER_STATE_READ_N 0x05
#define MASTER_STATE_ACKNOWLEDGEMENT 0x06
#define MASTER_STATE_FAILED 0x3F

#define MASTER_PIN_FROM_SLAVE 3
#define MASTER_PIN_TO_SLAVE 4

// Slave states. The two MSB must be zero since the bus is six bits.

#define SLAVE_STATE_READY 0x00
#define SLAVE_STATE_AWAITING_NUMBER_OF_BYTES_TO_WRITE 0x02
#define SLAVE_STATE_PRIMED_TO_WRITE_ONE_BYTE 0x03
#define SLAVE_STATE_PRIMED_TO_WRITE_N_BYTES 0x04
#define SLAVE_STATE_WRITING_TO_BUS 0x05
#define SLAVE_STATE_READING 0x06
#define SLAVE_STATE_RESETTING 0x07
#define SLAVE_STATE_INTERRUPTED 0x08
#define SLAVE_STATE_ERROR 0x3F

#define SLAVE_PIN_FROM_MASTER 3
#define SLAVE_PIN_TO_MASTER 4

class Master {
public:
    Master(uint8_t toSlave, uint8_t fromSlave, int pulseWidth_us);
	
	void enterWriteState();
    void requestAndReadOneByte(int delay_ms);
	void requestAndReadNBytes(int delay_ms, uint8_t N);
    void resetBus();
		
private:
    uint8_t _state;
    uint8_t _readByte;
	uint8_t _byteBuffer[64];
	uint8_t _numberOfBytesSuccessfullyRead;

    uint8_t _toSlaveMask;
	uint8_t _fromSlaveMask;
	
	int _pulseWidth_us;
	
	uint8_t _N;

	void takeControlOfBus();	
	void cedeControlOfBus();		
	
	// Building blocks
	void sendPulseToSlave(int delay_us);
	void sendCommandToSlaveAndWaitForAcknowledgement(uint8_t command, int delay_ms);
	
	// Synchronize with slave
	void readByteFromBus();	
	void sendSignalToSlaveAndReadFromBus(int delay_us);
	void synchronizedReadFromBus(int delay_us);	
	
};

class Slave {
public:
    Slave(uint8_t toMaster, uint8_t fromMaster, int pulseWidth_us);	
	void respondToInterrupt();
	
private:
    uint8_t _state;
    uint8_t _toMasterMask;
	uint8_t _fromMasterMask;
	
	int _pulseWidth_us;
	int _byteToBeWritten;
	int _bytesToBeWritten[255];

	void takeControlOfBus();	
	void cedeControlOfBus();	
	
	//These will be called during an interrupt sequence
	void mapCommandToState(uint8_t cmd);
	void writeOneByte(uint8_t data, int delay_ms);
	void writeNBytes(uint8_t* dataArray, uint8_t N, int delay_ms);
	void reset();
	
	 //Needed for testing
	uint8_t _counter;
	void readAndPrimeOneByteOfData();
	void readAndPrimeNBytesOfData(uint8_t N);
};

#endif //include guard
