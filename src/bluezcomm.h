/*
 * bluezcomm.h
 *
 *  Created on: Jan 9, 2015
 *      Author: freveny
 */

#ifndef SRC_BLUEZCOMM_H_
#define SRC_BLUEZCOMM_H_

int bluezcomm_init(char* btmac);
int bluezcomm_close(void);
int bluezcomm_media_playpause(void);
int bluezcomm_media_next(void);
int bluezcomm_media_prev(void);

#endif /* SRC_BLUEZCOMM_H_ */
