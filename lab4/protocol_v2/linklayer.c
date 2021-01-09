#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "timers.h"
#include "phy.h"
#include "net.h"
#include <errno.h>

// takes a frame, sets computes and sets its CRC
void computeCRC(Frame *frame)
{
	debugPrint("Computing CRC");
	//CRC-32 to be used which has ones in the following powers of x
	//32, 26, 23, 22, 16, 12, 11, 10, 8, 7, 5, 4, 2, 1, 0
	// since we know that highest power of G(x) is always 1, we choose to ommit it
	unsigned char G[4];
	// store such that G[0] stores bits 31-24 (most significant) and G[3] stores bits 7-0 (least significant)
	G[0] = (1 << (26 - 24));
	G[1] = (1 << (23 - 16)) + (1 << (22 - 16)) + (1 << (16 - 16));
	G[2] = (1 << (12 - 8)) + (1 << (11 - 8)) + (1 << (10 - 8)) + (1 << (8 - 8));
	G[3] = (1 << 7) + (1 << 5) + (1 << 4) + (1 << 2) + (1 << 1) + 1;

	// get framesize
	int frameSize = sizeof(Frame);
	// set initial crc = 0
	for (int i = 0; i < 4; i++)
		frame->crc[i] = 0;
	// copy frame as byte array for crc computation
	unsigned char *charFrame = (unsigned char *)malloc(sizeof(Frame));
	memcpy(charFrame, frame, frameSize);

	// remainder
	unsigned char remainder[4];
	// set initial remainder to high bits of dividend
	for (int i = 0; i < 4; i++)
		remainder[i] = charFrame[i];

	// modulo 2 division
	for (int i = 0; i + 32 < frameSize * 8; i++)
	{
		// get highest bit
		int high = (remainder[0] >> 7) & 1;
		// left shift by 1 bit
		for (int j = 0; j < 3; j++)
			remainder[j] = (remainder[j] << 1) | (remainder[j + 1] >> 7);
		remainder[3] = (remainder[3] << 1) | ((charFrame[(i / 8) + 4] >> (7 - (i % 8))) & 1);
		// xor if high is 1
		if (high == 1)
		{
			for (int j = 0; j < 4; j++)
			{
				remainder[j] ^= G[j];
			}
		}
	}

	// set crc = remainder
	for (int i = 0; i < 4; i++)
	{
		frame->crc[i] = remainder[i];
		// printf("%d ", remainder[i]);
	}
	// printf("\n");

	// free memory
	free(charFrame);
	// return remainder;
}

// takes a frame, and checks CRC, that is whether modulo 2 division is 0 or not
// returns true if 0, false otherwise
int checkCRC(Frame *frame)
{
	debugPrint("Checking CRC");
	//CRC-32 to be used which has ones in the following powers of x
	//32, 26, 23, 22, 16, 12, 11, 10, 8, 7, 5, 4, 2, 1, 0
	// since we know that highest power of G(x) is always 1, we choose to ommit it
	unsigned char G[4];
	// store such that G[0] stores bits 31-24 (most significant) and G[3] stores bits 7-0 (least significant)
	G[0] = (1 << (26 - 24));
	G[1] = (1 << (23 - 16)) + (1 << (22 - 16)) + (1 << (16 - 16));
	G[2] = (1 << (12 - 8)) + (1 << (11 - 8)) + (1 << (10 - 8)) + (1 << (8 - 8));
	G[3] = (1 << 7) + (1 << 5) + (1 << 4) + (1 << 2) + (1 << 1) + 1;

	// get framesize
	int frameSize = sizeof(Frame);
	// // extract crc
	// unsigned char crc[4];
	// for (int i = 0; i < 4; i++)
	// {
	// 	crc[i] = frame->crc[i];
	// 	frame->crc[i] = 0;
	// }
	// copy frame as byte array for crc computation
	unsigned char *charFrame = (unsigned char *)malloc(sizeof(Frame));
	memcpy(charFrame, frame, frameSize);

	// remainder
	unsigned char remainder[4];
	// set initial remainder to high bits of dividend
	for (int i = 0; i < 4; i++)
		remainder[i] = charFrame[i];

	// modulo 2 division
	for (int i = 0; i + 32 < frameSize * 8; i++)
	{
		// get highest bit
		int high = (remainder[0] >> 7) & 1;
		// left shift by 1 bit
		for (int j = 0; j < 3; j++)
			remainder[j] = (remainder[j] << 1) | (remainder[j + 1] >> 7);
		remainder[3] = (remainder[3] << 1) | ((charFrame[(i / 8) + 4] >> (7 - (i % 8))) & 1);
		// xor if high is 1
		if (high == 1)
		{
			for (int j = 0; j < 4; j++)
			{
				remainder[j] ^= G[j];
			}
		}
	}

	int flag = 1;

	// check crc is same as received
	// also reset the crc
	for (int i = 0; i < 4; i++)
	{
		// printf("%d, %d\n", remainder[i], frame->crc[i]);
		// if (remainder[i] != crc[i])
		if (remainder[i] != 0)
			flag = 0;
		// frame->crc[i] = crc[i];
	}

	// free memory
	free(charFrame);

	return flag;
}

// helper function to send ack
void sendAck(SeqNo seq)
{
	Frame f;
	f.frameKind = ACK;
	f.seq = -1;
	f.ack = seq;
	// set CRC
	computeCRC(&f);
	// send frame
	sendFrame(&f);
}

// helper function to send nack
void sendNack(SeqNo seq)
{
	Frame f;
	f.frameKind = NAK;
	f.seq = -1;
	f.ack = seq;
	// set CRC
	computeCRC(&f);
	// send frame
	sendFrame(&f);
}

int main(int argc, char **argv)
{
	Event event;
	initEvents();
	initTimers();
	initPhy(strtol(argv[2], NULL, 10), argv[1], strtol(argv[3], NULL, 10));
	initNet();
	SeqNo sendSeq = 0;
	SeqNo maxSend = -1;
	SeqNo recvSeq = 0;
	SeqNo maxRecv = -1;
	SeqNo windowSize = 100;
	// recv window of 100 frames max size
	Frame *recvBuffer[windowSize];
	// initialize with NULL
	for (SeqNo i = 0; i < windowSize; i++)
		recvBuffer[i] = NULL;
	// to denote how much of buffer is full
	SeqNo recvBufferFull = 0;
	// send window of 100 frames max size
	Frame *sendBuffer[windowSize];
	// initialize with NULL
	for (SeqNo i = 0; i < windowSize; i++)
		sendBuffer[i] = NULL;
	// to denote how much of buffer is full
	SeqNo sendBufferFull = 0;
	while (1)
	{
		waitForEvent(&event);

		switch (event.eventType)
		{
		case TIMEOUT:
			debugPrint("Timeout event occured");
			Timer *timer = (Timer *)event.eventData;
			printf("Timeout for SEQ no %d\n", timer->seqNo);
			// send again
			SeqNo repIndex = timer->seqNo % windowSize;
			// be cautious
			if (sendBuffer[repIndex] != NULL &&
				sendBuffer[repIndex]->seq == timer->seqNo)
			{
				sendFrame(sendBuffer[repIndex]);
				// restart timer
				startTimer(timer->seqNo);
			}
			// control shouldn't reach here
			else
				debugPrint("Timeout Error!");
			break;
		case ACK_TIMEOUT:
			debugPrint("ACK timeout event occured");

			break;
		case FRAME_ARRIVAL:
			debugPrint("Frame arrival event occured");
			Frame *fp = (Frame *)event.eventData;
			// check CRC
			if (checkCRC(fp) == 1)
			{
				debugPrint("CRC Verified!");
				// check frame kind
				switch (fp->frameKind)
				{
				case DATA:
					debugPrint("Data Received!");
					// if frame seq = recvSeq
					if (fp->seq == recvSeq)
					{
						// send data to network layer
						toNetworkLayer(fp);
						if (recvSeq > maxRecv)
							maxRecv = recvSeq;
						// increment recvSeq
						recvSeq++;
						// try to empty recvBuffer
						while (recvBufferFull > 0)
						{
							// get index
							SeqNo tempRecvBufferIndex = recvSeq % windowSize;
							// if that position contains data
							if (recvBuffer[tempRecvBufferIndex] != NULL &&
								recvBuffer[tempRecvBufferIndex]->seq == recvSeq)
							{
								// free(fp);
								fp = recvBuffer[tempRecvBufferIndex];
								recvBuffer[tempRecvBufferIndex] = NULL;
								recvBufferFull--;
								toNetworkLayer(fp);
								recvSeq++;
							}
							else
								break;
						}
						// send ack of largest received
						sendAck(recvSeq - 1);
					}
					// if frame seq > recvSeq
					else if (fp->seq > recvSeq)
					{
						// we have window of 100 frames, any frame ahead of that is discarded
						SeqNo offset = fp->seq - recvSeq;
						if (offset <= windowSize)
						{
							SeqNo saveRecvBufferIndex = fp->seq % windowSize;
							// check if this slot is empty
							if (recvBuffer[saveRecvBufferIndex] == NULL)
							{
								// save
								Frame *temp = (Frame *)malloc(sizeof(Frame));
								memcpy(temp, fp, sizeof(Frame));
								recvBuffer[saveRecvBufferIndex] = temp;
								recvBufferFull++;
								// if just next, update only
								if (temp->seq == maxRecv + 1)
									maxRecv++;
								// else if greater, update and nack
								else if (temp->seq > maxRecv)
								{
									maxRecv = temp->seq;
									sendNack(recvSeq);
								}
							}
						}
					}

					break;
				case ACK:
					debugPrint("ACK Received!");
					// free memory for all appropriate
					for (SeqNo i = maxSend + 1; i <= fp->ack; i++)
					{
						SeqNo index = i % windowSize;
						if (sendBuffer[index] != NULL &&
							sendBuffer[index]->seq == i)
						{
							free(sendBuffer[index]);
							sendBuffer[index] = NULL;
							sendBufferFull--;
						}
						// stop timer
						stopTimer(i);
					}
					// update maxSend
					if (fp->ack > maxSend)
						maxSend = fp->ack;
					break;
				case NAK:
					debugPrint("NAK received!");
					// send again
					SeqNo nakIndex = fp->ack % windowSize;
					// be cautious
					if (sendBuffer[nakIndex] != NULL &&
						sendBuffer[nakIndex]->seq == fp->ack)
					{
						sendFrame(sendBuffer[nakIndex]);
						// restart timer if required
						if (stopTimer(fp->ack) == 0)
							startTimer(fp->ack);
					}
					break;
				default:
					debugPrint("Unknown Frame Type Arrived!");
					break;
				}
			}
			// CRC failed, send NACK DATA
			else
				sendNack(recvSeq);

			break;
		case NETWORK_LAYER_READY:
			debugPrint("Network layer is ready");
			// index in send buffer
			SeqNo i = sendSeq % windowSize;
			// if can't save the sending buffer
			// reschedule the event
			if (sendBuffer[i] != NULL)
			{
				debugPrint("Sending window full! Cannot send!");
				enqueueEvent(event.eventType, event.eventData, event.cleanupFunc);
			}
			// else save and send frame
			else
			{
				Frame *f = (Frame *)malloc(sizeof(Frame));
				f->frameKind = DATA;
				f->seq = sendSeq;
				sendSeq++;
				f->ack = -1;
				// set data
				fromNetworkLayer(f);
				// set CRC
				computeCRC(f);
				// save frame
				Frame *temp = (Frame *)malloc(sizeof(Frame));
				memcpy(temp, f, sizeof(Frame));
				sendBuffer[i] = temp;
				sendBufferFull++;
				// send frame
				sendFrame(f);
				// start timer
				startTimer(sendSeq - 1);
			}
			break;
		default:
			debugPrint("Unknown event occured");
			break;
		}
		eventCleanup(&event);
	}

	return 0;
}
