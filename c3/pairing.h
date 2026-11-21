#ifndef C3_PAIRING_H__
#define C3_PAIRING_H__

//Peer and serial commands
void initPeer(void);
bool hasPeer(void);
const char *getPeer(void);
bool setPeer(const char *addr);
void clearPeer(void);
bool loopSerial(void);

#endif /* C3_PAIRING_H__ */
