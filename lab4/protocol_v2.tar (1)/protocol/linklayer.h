#define MAX_PKT 1024
#define MAX_SEQ 7
#define inc(k) if (k < MAX_SEQ) k = k + 1; else k = 0

typedef enum {DATA, ACK, NAK} FrameKind;
typedef unsigned int SeqNo;
typedef struct { unsigned char data[MAX_PKT]; } Packet;
typedef enum {true, false} bool;



typedef struct Frame_t {
	FrameKind frameKind;
	SeqNo seq;
	SeqNo ack;
	unsigned char length[2];
	unsigned char padding[2];
	Packet packet;
	unsigned char crc[4];
} Frame;

