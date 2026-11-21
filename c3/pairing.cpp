#include <Arduino.h>
#include <Preferences.h>

#include "pairing.h"
#include "main.h"

static Preferences prefs;
static char peerAddr[ADDR_LEN];

static char serialBuf[MSG_LEN];
static size_t serialLen = 0;

//Accept "XX:XX:XX:XX:XX:XX" and normalize to lower case
static bool parseAddr(const char *in, char *out, size_t len) {
  unsigned int b[6];
  char tail;
  int r;

  r = sscanf(in, "%2x:%2x:%2x:%2x:%2x:%2x%c",
      &b[0], &b[1], &b[2], &b[3], &b[4], &b[5], &tail);
  if (r != 6) {
    return false;
  }

  snprintf(out, len, "%02x:%02x:%02x:%02x:%02x:%02x",
      b[0], b[1], b[2], b[3], b[4], b[5]);

  return true;
}

void initPeer(void) {
  prefs.begin("remote", false);

  String s = prefs.getString("peer", "");
  if (!parseAddr(s.c_str(), peerAddr, sizeof(peerAddr))) {
    peerAddr[0] = '\0';
  }
}

bool hasPeer(void) {
  return peerAddr[0] != '\0';
}

const char *getPeer(void) {
  return peerAddr;
}

bool setPeer(const char *addr) {
  char buf[ADDR_LEN];

  if (!parseAddr(addr, buf, sizeof(buf))) {
    return false;
  }

  strlcpy(peerAddr, buf, sizeof(peerAddr));
  prefs.putString("peer", peerAddr);

  return true;
}

void clearPeer(void) {
  peerAddr[0] = '\0';
  prefs.remove("peer");
}

static void printPeer(void) {
  if (hasPeer()) {
    Serial.printf("peer: %s\n", getPeer());
  } else {
    Serial.printf("peer: (none)\n");
  }
}

//Return true if the peer is changed
static bool doCommand(const char *cmd) {
  if (strcmp(cmd, "peer") == 0) {
    printPeer();
    return false;
  }

  if (strncmp(cmd, "peer set ", 9) == 0) {
    if (setPeer(cmd + 9)) {
      printPeer();
      return true;
    }

    Serial.printf("peer set: invalid address '%s'\n", cmd + 9);
    return false;
  }

  if (strcmp(cmd, "peer clear") == 0) {
    clearPeer();
    printPeer();
    return true;
  }

  if (strcmp(cmd, "help") == 0) {
    Serial.printf("peer                         : Show the receiver address.\n");
    Serial.printf("peer set XX:XX:XX:XX:XX:XX   : Set the receiver address.\n");
    Serial.printf("peer clear                   : Clear the receiver address.\n");
    Serial.printf("  ver." VERSION "\n");
    return false;
  }

  if (cmd[0] != '\0') {
    Serial.printf("unknown command '%s'\n", cmd);
  }

  return false;
}

//Return true if the peer is changed
bool loopSerial(void) {
  bool changed = false;

  while (Serial.available()) {
    int c = Serial.read();

    if (c == '\r' || c == '\n') {
      serialBuf[serialLen] = '\0';
      changed |= doCommand(serialBuf);
      serialLen = 0;
    } else if (serialLen < sizeof(serialBuf) - 1) {
      serialBuf[serialLen++] = c;
    }
  }

  return changed;
}
