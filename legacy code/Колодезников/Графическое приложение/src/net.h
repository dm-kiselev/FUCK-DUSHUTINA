#ifndef NET_H_
#define NET_H_

#include <../../IndividualProject/datatypes.h>
int frame_send(int coid, frame_t* psframe, frame_t* prframe);
int frame_datareceive(int rcvid, frame_t *pframe);
int frame_reply(int rcvid, frame_t* pframe);
void frame_repinit(frame_t* psframe, frame_t *prframe);
void frame_repair(frame_t *pframe, iov_t *piov);
void frame_destroy(frame_t* pframe);

#endif /* NET_H_ */
